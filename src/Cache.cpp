#include "Cache.h"
#include "RAM.h"
#include "CPU.h"
#include "Constants.h"
#include <iostream>
#include <stdexcept>

RAM* Cache::ram = nullptr;

Cache::Cache(int numBlocks, int w) : size(numBlocks * BLOCK_SIZE), ways(w), numSets(0) {
    if (numBlocks <= 0 || w <= 0) throw std::runtime_error("Invalid cache config");
    if (numBlocks % w != 0) throw std::runtime_error("Blocks must be divisible by ways");
    numSets = numBlocks / w;
    mem.assign(static_cast<size_t>(numSets) * static_cast<size_t>(ways) * BLOCK_SIZE, 0);
    tags.assign(static_cast<size_t>(numSets), std::map<int, CacheLineInfo>{});
    setLocks = std::vector<std::mutex>(static_cast<size_t>(numSets));
    dirty.assign(static_cast<size_t>(numSets), std::vector<char>(static_cast<size_t>(ways), 0));
}

Cache* Cache::createInstance(int s, int w) {
  instance = std::unique_ptr<Cache>(new Cache(s, w));
  return instance.get();
}

void Cache::resetStats() {
    cacheHits = 0;
    cacheMisses = 0;
    ramReads = 0;
    ramWrites = 0;
    tags.clear();
    tags.assign(static_cast<size_t>(numSets), std::map<int, CacheLineInfo>{});
    dirty.clear();
    dirty.assign(static_cast<size_t>(numSets), std::vector<char>(static_cast<size_t>(ways), 0));
    init();
}


void Cache::init() {
    int numCores = CPU::getInstance()->getNumCores();
    lastWrite.clear();
    lastWrite.resize(static_cast<size_t>(numSets), std::vector<int>(static_cast<size_t>(numCores), -1));

    int waysPerCore = ways / numCores;
    if (partitioningEnabled) {
        for (int s = 0; s < numSets; ++s) {
            for (int c = 0; c < numCores; ++c)
                lastWrite[s][c] = c * waysPerCore - 1;
        }
    } else {
        for (int s = 0; s < numSets; ++s) {
            for (int c = 0; c < numCores; ++c)
                lastWrite[s][c] = -1;
        }
    }
    ram = RAM::getInstance();
}

int Cache::get(int addr, int coreId) {
    if (numSets == 0) throw std::runtime_error("Cache not initialized");

    int blockNum = addr / BLOCK_SIZE;
    int setIndex = blockNum % numSets;
    int tag = blockNum / numSets;

    std::lock_guard<std::mutex> lock(setLocks[setIndex]);

    auto it = tags[setIndex].find(tag);
    if (it == tags[setIndex].end() ||
        (!partitioningEnabled && tags[setIndex][tag].ownerCore != coreId)) {
        cacheMisses++;
        copyBlockUnlocked(blockNum, coreId); // no re-lock
        it = tags[setIndex].find(tag);
        if (it == tags[setIndex].end())
            throw std::runtime_error("[Cache::get] Miss handling failed");
    } else {
        cacheHits++;
    }

    int way = it->second.wayIndex;
    return mem[(way * numSets + setIndex) * BLOCK_SIZE + (addr % BLOCK_SIZE)];
}

void Cache::set(int addr, int val, int coreId) {
    if (numSets == 0) throw std::runtime_error("Cache not initialized");

    int blockNum = addr / BLOCK_SIZE;
    int setIndex = blockNum % numSets;
    int tag = blockNum / numSets;

    std::lock_guard<std::mutex> lock(setLocks[setIndex]);

    auto it = tags[setIndex].find(tag);
    if (it == tags[setIndex].end() ||
        (!partitioningEnabled && tags[setIndex][tag].ownerCore != coreId)) {
        cacheMisses++;
        copyBlockUnlocked(blockNum, coreId); // no re-lock
        it = tags[setIndex].find(tag);
        if (it == tags[setIndex].end())
            throw std::runtime_error("[Cache::set] Miss handling failed");
    } else {
        cacheHits++;
    }

    int way = it->second.wayIndex;
    mem[(way * numSets + setIndex) * BLOCK_SIZE + (addr % BLOCK_SIZE)] = val;
    dirty[setIndex][way] = true;
}

void Cache::copyBlock(int blockNum, int coreId) {
    // Keep public version for safety
    int setIndex = blockNum % numSets;
    std::lock_guard<std::mutex> lock(setLocks[setIndex]);
    copyBlockUnlocked(blockNum, coreId);
}

void Cache::copyBlockUnlocked(int blockNum, int coreId) {
    int setIndex = blockNum % numSets;

    int targetWay;
    if (partitioningEnabled) {
        int waysPerCore = ways / CPU::getInstance()->getNumCores();
        lastWrite[setIndex][coreId]++;
        int partitionStart = coreId * waysPerCore;
        if (lastWrite[setIndex][coreId] - partitionStart >= waysPerCore)
            lastWrite[setIndex][coreId] = partitionStart;
        targetWay = lastWrite[setIndex][coreId];
    } else {
        lastWrite[setIndex][coreId]++;
        if (lastWrite[setIndex][coreId] >= ways) lastWrite[setIndex][coreId] = 0;
        targetWay = lastWrite[setIndex][coreId];
    }

    int newTag = blockNum / numSets;

    // Evict old line
    int oldTag = -1;
    for (auto it = tags[setIndex].begin(); it != tags[setIndex].end(); ++it) {
        if (it->second.wayIndex == targetWay) {
            oldTag = it->first;
            tags[setIndex].erase(it);
            break;
        }
    }

    if (oldTag != -1 && dirty[setIndex][targetWay]) {
        int evictedBlockNum = oldTag * numSets + setIndex;
        size_t startAddr = (targetWay * numSets + setIndex) * BLOCK_SIZE;
        for (int i = 0; i < BLOCK_SIZE; ++i)
            ram->mem[evictedBlockNum * BLOCK_SIZE + i] = mem[startAddr + i];
        dirty[setIndex][targetWay] = 0;
        ramWrites += BLOCK_SIZE;
    }

    tags[setIndex][newTag] = {targetWay, coreId};

    size_t startAddrMem = (targetWay * numSets + setIndex) * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        mem[startAddrMem + i] = ram->mem[blockNum * BLOCK_SIZE + i];
    }
    dirty[setIndex][targetWay] = 0;
    ramReads += BLOCK_SIZE;
}
