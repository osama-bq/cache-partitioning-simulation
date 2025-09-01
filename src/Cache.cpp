#include "Cache.h"
#include "RAM.h"
#include "CPU.h"
#include "Utilities.h"
#include "Constants.h"

#include <iostream>
#include <stdexcept>
#include <cassert>

RAM* Cache::ram = nullptr;

Cache::Cache(int numBlocks, int w) :
  size(numBlocks * BLOCK_SIZE),
  ways(w),
  numSets(0),
  mem() {
  if (numBlocks <= 0) throw std::runtime_error("Cache must have positive number of blocks");
  if (w <= 0) throw std::runtime_error("Cache ways must be positive");
  if (numBlocks % w != 0) {
    throw std::runtime_error("Number of blocks must be divisible by ways (numBlocks % ways != 0)");
  }
  numSets = numBlocks / w;
  mem.assign(static_cast<size_t>(ways) * static_cast<size_t>(numSets) * BLOCK_SIZE, 0);
  tags.assign(static_cast<size_t>(numSets), std::map<int,int>{});
  // allocate per-set structures now that numSets is known
  setLocks = std::vector<std::mutex>(static_cast<size_t>(numSets));
  dirty.assign(static_cast<size_t>(numSets), std::vector<char>(static_cast<size_t>(ways), 0));
}

void Cache::init() {
  if (numSets <= 0)
    throw std::runtime_error("Cache::init called before proper construction");

  int numCores = CPU::getInstance()->getNumCores();
  if (ways < numCores)
    throw std::runtime_error("Cache::init: ways < numCores - this implementation requires ways >= numCores");

  int waysPerCore = ways / numCores;
  if (waysPerCore <= 0)
    throw std::runtime_error("Cache::init: computed waysPerCore <= 0");

  lastWrite.clear();
  lastWrite.resize(static_cast<size_t>(numSets), std::vector<int>(static_cast<size_t>(numCores), -1));

  for (int s = 0; s < numSets; ++s) {
    for (int c = 0; c < numCores; ++c) {
      lastWrite[s][c] = c * waysPerCore - 1;
    }
  }

  ram = RAM::getInstance();
}

Cache* Cache::createInstance(int s, int w) {
  instance = std::unique_ptr<Cache>(new Cache(s, w));
  return instance.get();
}

bool Cache::check(int addr) {
  if (addr < 0) throw std::runtime_error("Cache::check: negative address");
  if (numSets == 0)
    throw std::runtime_error("Cache not initialized (numSets == 0)");
  int blockNum = addr / BLOCK_SIZE;
  int setIndex = blockNum % numSets;
  int tag = blockNum / numSets;

  if (setIndex < 0 || setIndex >= numSets)
    throw std::runtime_error("Cache::check: setIndex out of range");

  std::lock_guard<std::mutex> lock(setLocks[static_cast<size_t>(setIndex)]);
  bool hit = tags[setIndex].count(tag) > 0;
  std::cout << "Checking cache for address " << addr << " (block " << blockNum
            << ", set " << setIndex << ", tag " << tag << "): "
            << (hit ? "HIT" : "MISS") << std::endl;
  return hit;
}

int Cache::get(int addr) {
  if (addr < 0) throw std::runtime_error("Cache::get: negative address");
  if (numSets == 0) throw std::runtime_error("Cache not initialized");

  int blockNum = addr / BLOCK_SIZE;
  int setIndex = blockNum % numSets;
  int tag = blockNum / numSets;
  int offset = addr % BLOCK_SIZE;

  std::lock_guard<std::mutex> lock(setLocks[static_cast<size_t>(setIndex)]);

  if (!tags[setIndex].count(tag))
    throw std::runtime_error("[Cache::get] Cache miss at address " + std::to_string(addr));

  int wayIndex = tags[setIndex].at(tag);
  if (wayIndex < 0 || wayIndex >= ways)
    throw std::runtime_error("[Cache::get] Invalid way index stored in tags");

  size_t memIndex = (static_cast<size_t>(wayIndex) * static_cast<size_t>(numSets) + static_cast<size_t>(setIndex)) * BLOCK_SIZE + static_cast<size_t>(offset);
  if (memIndex >= mem.size()) throw std::runtime_error("[Cache::get] mem index out of range");

  return mem[memIndex];
}

void Cache::set(int addr, int val) {
  if (addr < 0) throw std::runtime_error("Cache::set: negative address");
  if (numSets == 0) throw std::runtime_error("Cache not initialized");
  int blockNum = addr / BLOCK_SIZE;
  int setIndex = blockNum % numSets;
  int tag = blockNum / numSets;
  int offset = addr % BLOCK_SIZE;

  std::lock_guard<std::mutex> lock(setLocks[static_cast<size_t>(setIndex)]);

  if (!tags[setIndex].count(tag))
    throw std::runtime_error("[Cache::set] Cache miss at address " + std::to_string(addr));

  int wayIndex = tags[setIndex][tag];
  if (wayIndex < 0 || wayIndex >= ways)
    throw std::runtime_error("[Cache::set] Invalid way index stored in tags");

  size_t memIndex = (static_cast<size_t>(wayIndex) * static_cast<size_t>(numSets) + static_cast<size_t>(setIndex)) * BLOCK_SIZE + static_cast<size_t>(offset);
  if (memIndex >= mem.size()) throw std::runtime_error("[Cache::set] mem index out of range");

  mem[memIndex] = val;
  dirty[setIndex][wayIndex] = 1; // mark dirty
}

void Cache::copyBlock(int blockNum, int coreId) {
  if (numSets == 0) throw std::runtime_error("Cache not initialized");
  if (blockNum < 0) throw std::runtime_error("Cache::copyBlock: negative blockNum");
  int numCores = CPU::getInstance()->getNumCores();
  if (coreId < 0 || coreId >= numCores) throw std::runtime_error("Cache::copyBlock: invalid coreId");

  int setIndex = blockNum % numSets;
  if (setIndex < 0 || setIndex >= numSets) throw std::runtime_error("Cache::copyBlock: setIndex out of range");

  // lock only this set
  std::lock_guard<std::mutex> lock(setLocks[static_cast<size_t>(setIndex)]);

  int prev = lastWrite[setIndex][coreId];
  // advance to next way in the core's partition
  lastWrite[setIndex][coreId]++;

  int waysPerCore = ways / numCores;
  if (waysPerCore <= 0) throw std::runtime_error("Cache::copyBlock: waysPerCore <= 0");

  int partitionStart = coreId * waysPerCore;
  if (lastWrite[setIndex][coreId] - partitionStart >= waysPerCore)
    lastWrite[setIndex][coreId] = partitionStart;

  int targetWay = lastWrite[setIndex][coreId];

  int newTag = blockNum / numSets;

  // find old tag that occupied this way (if any)
  int oldTag = -1;
  for (auto it = tags[setIndex].begin(); it != tags[setIndex].end(); ++it) {
    if (it->second == targetWay) {
      oldTag = it->first;
      tags[setIndex].erase(it);
      break;
    }
  }

  // if the old line exists and is dirty, write it back to its RAM block
  if (oldTag != -1 && dirty[setIndex][targetWay]) {
    int evictedBlockNum = oldTag * numSets + setIndex;
    size_t prevStart = (static_cast<size_t>(targetWay) * static_cast<size_t>(numSets) + static_cast<size_t>(setIndex)) * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
      ram->mem[static_cast<size_t>(evictedBlockNum) * BLOCK_SIZE + static_cast<size_t>(i)] = mem[prevStart + static_cast<size_t>(i)];
    }
    dirty[setIndex][targetWay] = 0;
  }

  // install new tag -> way mapping
  tags[setIndex][newTag] = targetWay;

  std::cout << "Core " << coreId << " loading block " << blockNum
    << "( byte range " << (blockNum * BLOCK_SIZE) << " ~ " << ((blockNum + 1) * BLOCK_SIZE - 1) << " )"
    << " into cache set " << setIndex << " (way " << targetWay << ")\n";

  size_t startAddrMem = (static_cast<size_t>(targetWay) * static_cast<size_t>(numSets) + static_cast<size_t>(setIndex)) * BLOCK_SIZE;

  // load block from RAM into cache way
  for (int i = 0; i < BLOCK_SIZE; ++i) {
    mem[startAddrMem + static_cast<size_t>(i)] = ram->mem[static_cast<size_t>(blockNum) * BLOCK_SIZE + static_cast<size_t>(i)];
  }
  // freshly loaded line is clean
  dirty[setIndex][targetWay] = 0;
}
