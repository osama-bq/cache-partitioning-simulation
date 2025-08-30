#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <map>
#include "HardwareComponent.h"
#include "RAM.h"

class Cache : public HardwareComponent<Cache> {
  friend class HardwareComponent<Cache>;
  friend class Core;
  static RAM* ram;
  int size, ways;
  Cache(int = 1, int = 2);
protected:
  std::vector<std::vector<int>> lastWrite;
  std::vector<std::map<int, int>> tags;
  std::vector<int> mem;
  public:
  void init();
  static Cache* createInstance(int, int);
  bool check(int) const;
  int get(int) const;
  void set(int, int);
  void copyBlock(int, int);
};

#endif