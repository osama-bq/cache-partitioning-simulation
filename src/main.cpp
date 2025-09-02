#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <iomanip>  // for std::setw, std::setprecision
#include <memory>   // for std::unique_ptr
#include "OS.h"
#include "Cache.h"
#include "Process.h"
#include "Constants.h"

// -----------------------------
// Simple energy model constants
// -----------------------------
struct EnergyModel {
    double E_cache_access = 1.0;   // per cache access (hit or miss)
    double E_ram_read     = 50.0;  // per RAM read
    double E_ram_write    = 70.0;  // per RAM write
};

struct RunStats {
    int hits = 0;
    int misses = 0;
    int ramReads = 0;
    int ramWrites = 0;
    double hitRate = 0.0;
    double energy = 0.0;
};

static RunStats collectStats(Cache* cache, const EnergyModel& em) {
    RunStats s;
    s.hits = cache->getCacheHits();
    s.misses = cache->getCacheMisses();
    s.ramReads = cache->getRamReads();
    s.ramWrites = cache->getRamWrites();

    const int totalCacheAccesses = s.hits + s.misses;
    s.hitRate = (totalCacheAccesses > 0)
        ? static_cast<double>(s.hits) / static_cast<double>(totalCacheAccesses)
        : 0.0;

    const double Ecache = em.E_cache_access * static_cast<double>(totalCacheAccesses);
    const double Eram   = em.E_ram_read * static_cast<double>(s.ramReads)
                        + em.E_ram_write * static_cast<double>(s.ramWrites);
    s.energy = Ecache + Eram;
    return s;
}

std::vector<Process> processes;
// -------------------------------------------
// Build exactly numCores processes so that
// sum(processSizes) <= ramCapacityBytes.
// Each process size = 2*numInstr + dataSpace.
// We choose a total process size first, then
// split it into (numInstr, dataSpace).
// -------------------------------------------
static void makeProcessesForRam(
    int numCores,
    int ramBlocks,
    std::mt19937& rng,
    int minProcSize,   // inclusive, in "words"
    int maxProcSize    // inclusive, in "words"
) {
    processes.reserve(static_cast<size_t>(numCores));

    const int ramCapacity = ramBlocks * BLOCK_SIZE; // total "words" in RAM (match your unit)

    // Make a conservative per-process cap so we don't overrun RAM.
    // We leave a small headroom margin to accommodate allocator metadata or fragmentation.
    const int safeTotal = static_cast<int>(ramCapacity * 0.9);
    const int maxPerProcess = std::max(minProcSize, safeTotal / numCores);

    std::uniform_int_distribution<int> procSizeDist(minProcSize, std::min(maxProcSize, maxPerProcess));
    std::uniform_real_distribution<double> splitRatio(0.3, 0.7); // portion for (2*numInstr) vs dataSpace

    int totalSoFar = 0;
    for (int i = 0; i < numCores; ++i) {
        int size = procSizeDist(rng);

        // If this would exceed safeTotal, clamp it.
        if (totalSoFar + size > safeTotal) {
            size = std::max(minProcSize, safeTotal - totalSoFar);
        }
        if (size < minProcSize) size = minProcSize; // final clamp if needed

        // Split "size = 2*numInstr + dataSpace"
        double r = splitRatio(rng);
        int twoNI = static_cast<int>(r * size);
        if (twoNI % 2 != 0) ++twoNI; // ensure even so numInstr is integer
        if (twoNI <= 0) twoNI = 2;

        int numInstr = std::max(1, twoNI / 2);
        int dataSpace = std::max(1, size - 2 * numInstr);

        // Re-adjust if tiny mismatch pushed us out of bounds
        while (2 * numInstr + dataSpace > size) {
            if (dataSpace > 1) --dataSpace;
            else if (numInstr > 1) --numInstr;
            else break;
        }

        // Create via API
        Process p = Process::createRandom(numInstr, dataSpace);
        processes.emplace_back(std::move(p));

        totalSoFar += (2 * numInstr + dataSpace);
    }
}

// -------------------------------------------
// Run one simulation (either partitioned or not)
// - Loads the same set of processes
// - Starts one process per core in parallel
// - Waits for all to finish
// -------------------------------------------
static RunStats runOneMode(
    OS *os,
    bool partitioningEnabled,
    const EnergyModel& em
) {
    // Toggle partitioning on this OS's cache
    os->getCache()->setPartitioning(partitioningEnabled);

    // Load processes, one per core
    const int numCores = os->getCPU()->getNumCores();
    for (int i = 0; i < numCores; ++i)
        os->loadProcess(processes[static_cast<size_t>(i)]);

    // Launch each on its own core; collect threads
    std::vector<std::thread> workers;
    workers.reserve(static_cast<size_t>(numCores));
    auto& cores = os->getCPU()->getCores();  // reference, not copy
    for (int i = 0; i < numCores; ++i)
        workers.emplace_back(cores[i].runProcess(processes[static_cast<size_t>(i)]));


    // Wait for all to complete
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    // Collect stats from cache
    return collectStats(os->getCache(), em);
}

int main() {
    // -----------------------------
    // Default configuration chosen
    // to make partitioning useful:
    // - more cores than ways will
    //   create contention in shared mode
    // -----------------------------
    const int cpuCores   = 4;
    const int ramBlocks  = 1024;  // RAM blocks
    const int cacheBlocks= 32;   // total cache blocks
    const int cacheWays  = 8;    // low associativity to amplify contention

    // Process sizing:
    // We want processes large enough to churn the cache,
    // but not so large that RAM fills up.
    const int minProcSize = 200;  // in "words"
    const int maxProcSize = 400;  // in "words"

    // Energy model
    EnergyModel em; // use defaults

    std::mt19937 rng(std::random_device{}());

    // Build one set of processes to be reused in both runs
    makeProcessesForRam(cpuCores, ramBlocks, rng, minProcSize, maxProcSize);
    if (static_cast<int>(processes.size()) != cpuCores) {
        std::cerr << "Failed to create exactly numCores processes.\n";
        return 1;
    }

    // -----------------------------
    // Run #1: NO PARTITIONING
    // Fresh OS so cache/RAM are clean
    // -----------------------------
    auto os = OS::createInstance(cpuCores, ramBlocks, cacheBlocks, cacheWays);
    auto cache = os->getCache();
    auto ram = os->getRAM();

    RunStats sShared = runOneMode(os, /*partitioningEnabled=*/false, em);

    cache->resetStats(); // clear stats before next run
    ram->clear();      // clear RAM contents before next run

    // -----------------------------
    // Run #2: PARTITIONING ENABLED
    // Fresh OS again (same params)
    // -----------------------------
    RunStats sPart = runOneMode(os, /*partitioningEnabled=*/true, em);

    // -----------------------------
    // Summary
    // -----------------------------
    std::cout << "\n================= Simulation Summary =================\n";
    std::cout << "Cores: " << cpuCores
              << " | RAM blocks: " << ramBlocks
              << " | Cache blocks: " << cacheBlocks
              << " | Ways: " << cacheWays << "\n\n";

    auto printRow = [](const std::string& label, const std::string& shared, const std::string& part) {
        std::cout << std::left << std::setw(24) << label
                  << std::setw(20) << shared
                  << std::setw(20) << part << "\n";
    };

    std::cout << std::left << std::setw(24) << ""
              << std::setw(20) << "No Partitioning"
              << std::setw(20) << "Partitioning" << "\n";
    std::cout << std::string(64, '-') << "\n";

    printRow("Cache Hits",
             std::to_string(sShared.hits),
             std::to_string(sPart.hits));

    printRow("Cache Misses",
             std::to_string(sShared.misses),
             std::to_string(sPart.misses));

    const auto rateStr = [](double r) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << (r * 100.0) << "%";
        return oss.str();
    };

    printRow("Hit Rate",
             rateStr(sShared.hitRate),
             rateStr(sPart.hitRate));

    printRow("RAM Reads",
             std::to_string(sShared.ramReads),
             std::to_string(sPart.ramReads));

    printRow("RAM Writes",
             std::to_string(sShared.ramWrites),
             std::to_string(sPart.ramWrites));

    auto energyStr = [](double e) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << e;
        return oss.str();
    };

    printRow("Energy (arbitrary)",
             energyStr(sShared.energy),
             energyStr(sPart.energy));

    std::cout << "======================================================\n\n";

    return 0;
}
