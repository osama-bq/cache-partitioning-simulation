#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <map>
#include <mutex>
#include "HardwareComponent.h"
#include "RAM.h"

class Cache : public HardwareComponent<Cache> {
    friend class HardwareComponent<Cache>;
    friend class Core;

    static RAM* ram;
    int size;       // total cache size in bytes = numBlocks * BLOCK_SIZE
    int ways;       // associativity
    int numSets;    // number of sets
    bool partitioningEnabled = true; // partitioning toggle

    Cache(int = 1, int = 2);

    void copyBlockUnlocked(int blockNum, int coreId);

protected:
    struct CacheLineInfo {
        int wayIndex;
        int ownerCore; // -1 if free or shared
    };

    std::vector<std::vector<int>> lastWrite;               // lastWrite[set][core]
    std::vector<std::map<int, CacheLineInfo>> tags;        // tags[set][tag] -> CacheLineInfo
    std::vector<int> mem;                                  // linear storage
    std::vector<std::mutex> setLocks;                      // per-set locks
    std::vector<std::vector<char>> dirty;                 // dirty[set][way]

    // Counters
    int cacheHits = 0;
    int cacheMisses = 0;
    int ramReads = 0;
    int ramWrites = 0;

public:
    void init();
    static Cache* createInstance(int, int);

    // API
    int get(int addr, int coreId);
    void set(int addr, int val, int coreId);
    void copyBlock(int blockNum, int coreId);
    void resetStats();

    void setPartitioning(bool enable) { partitioningEnabled = enable; }
    bool isPartitioned() const { return partitioningEnabled; }

    // Stats
    int getCacheHits() const { return cacheHits; }
    int getCacheMisses() const { return cacheMisses; }
    int getRamReads() const { return ramReads; }
    int getRamWrites() const { return ramWrites; }

    // Helpers
    int getNumSets() const { return numSets; }
    int getWays() const { return ways; }
    int getSizeBytes() const { return size; }
};

#endif
