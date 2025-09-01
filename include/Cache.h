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
  int ways;       // associativity (number of ways)
  int numSets;    // number of sets in the cache
  Cache(int = 1, int = 2);
protected:
  // lastWrite[setIndex][coreId] -> last way index used by that core in that set
  std::vector<std::vector<int>> lastWrite;
  // tags[setIndex] maps tag -> wayIndex
  std::vector<std::map<int, int>> tags;
  // linear storage: (way * numSets + setIndex) * BLOCK_SIZE + offset
  std::vector<int> mem;

  // per-set mutexes for concurrency
  std::vector<std::mutex> setLocks;
  // dirty[setIndex][wayIndex] flag (char for compactness)
  std::vector<std::vector<char>> dirty;

public:
  void init();
  static Cache* createInstance(int, int);
  bool check(int);
  int get(int);
  void set(int, int);
  void copyBlock(int, int); // blockNum, coreId
  // helpers
  int getNumSets() const { return numSets; }
  int getWays() const { return ways; }
  int getSizeBytes() const { return size; }
};

#endif
