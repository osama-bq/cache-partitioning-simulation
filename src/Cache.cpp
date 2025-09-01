#include "Cache.h"
#include "RAM.h"
#include "CPU.h"
#include "Utilities.h"
#include "Constants.h"

#include <iostream>

RAM* Cache::ram = nullptr;

Cache::Cache(int numBlocks, int w) :
  size(numBlocks * BLOCK_SIZE),
  ways(w),
  mem(size),
  tags(size / ways) {}

void Cache::init() {
  int numSets = size / ways;
  int numCores = CPU::getInstance()->getNumCores();
  lastWrite.resize(numSets, std::vector<int>(numCores, -1));
  ram = RAM::getInstance();
}

Cache* Cache::createInstance(int s, int w) {
  instance = std::unique_ptr<Cache>(new Cache(s, w));
  return instance.get();
}

bool Cache::check(int addr) const {
  int numSets = size / (ways * BLOCK_SIZE);
  if (numSets == 0)
    throw std::runtime_error("Cache not initialized");
  int setIndex = (addr / BLOCK_SIZE) % numSets;
  int tag = addr / (numSets * BLOCK_SIZE);

  std::cout << "Checking cache for address " << addr << " (set " << setIndex << ", tag " << tag << "): ";
  std::cout << (tags[setIndex].count(tag) ? "HIT" : "MISS") << std::endl;
  return tags[setIndex].count(tag) > 0;
}

int Cache::get(int addr) const {
  int numSets = size / (ways * BLOCK_SIZE);
  if (numSets == 0)
    throw std::runtime_error("Cache not initialized");
  int setIndex = (addr / BLOCK_SIZE) % numSets;
  int tag = addr / (numSets * BLOCK_SIZE);

  if (!tags[setIndex].count(tag))
    throw std::runtime_error("[Cache::get] Cache miss at address " + std::to_string(addr));
  return mem[(tags[setIndex].at(tag) * numSets + setIndex) * BLOCK_SIZE + (addr % BLOCK_SIZE)];
}

void Cache::set(int addr, int val) {
  int numSets = size / (ways * BLOCK_SIZE);
  int setIndex = (addr / BLOCK_SIZE) % numSets;
  if (numSets == 0)
    throw std::runtime_error("Cache not initialized");
  int tag = addr / (numSets * BLOCK_SIZE);

  if (!tags[setIndex].count(tag))
    throw std::runtime_error("[Cache::set] Cache miss at address " + std::to_string(addr));
  
  mem[(tags[setIndex][tag] * numSets + setIndex) * BLOCK_SIZE + (addr % BLOCK_SIZE)] = val;
}

void Cache::copyBlock(int blockNum, int coreId) {
  int numSets = size / (ways * BLOCK_SIZE);
  if (numSets == 0)
    throw std::runtime_error("Cache not initialized");
  int numCores = CPU::getInstance()->getNumCores();
  int setIndex = blockNum % numSets;

  int prev = lastWrite[setIndex][coreId];
  lastWrite[setIndex][coreId]++;

  int waysPerCore = ways / numCores;
  if (lastWrite[setIndex][coreId] - coreId * waysPerCore >= waysPerCore)
    lastWrite[setIndex][coreId] = coreId * waysPerCore;

  int tag = blockNum / numSets;
  if (tags[setIndex].count(tag))
    throw std::runtime_error("Block already in cache");
    
  for (auto it = tags[setIndex].begin(); it != tags[setIndex].end(); ++it) {
    if (it->second == prev) {
      tags[setIndex].erase(it);
      break;
    }
  }
  tags[setIndex][tag] = lastWrite[setIndex][coreId];

  std::cout << "Core " << coreId << " loading block " << blockNum
    << "( " << blockNum * BLOCK_SIZE << " ~ " << ((blockNum + 1) * BLOCK_SIZE) - 1 << " )"
    << " into cache set " << setIndex << " (way " << lastWrite[setIndex][coreId] << ")\n";

  int startAddr = (lastWrite[setIndex][coreId] * numSets + setIndex) * BLOCK_SIZE;
  for (int i = 0; i < BLOCK_SIZE; i++) {
    if (prev != -1)
      ram->mem[blockNum * BLOCK_SIZE + i] = mem[startAddr + i]; // write back
    mem[startAddr + i] = ram->mem[blockNum * BLOCK_SIZE + i];
  }
}