#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include "HardwareComponent.h"

class Cache : HardwareComponent<Cache> {
  friend class HardwareComponent<Cache>;
  int size, ways;
  std::vector<char> mem;
  Cache(int s = 4, int w = 2);
public:
  static Cache* createInstance(int, int);
};

#endif