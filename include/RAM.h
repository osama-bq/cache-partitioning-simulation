#ifndef RAM_H
#define RAM_H

#include <vector>
#include <queue>
#include "HardwareComponent.h"

class RAM : public HardwareComponent<RAM> {
  friend class HardwareComponent<RAM>;
  friend class OS;
  friend class Core;
  friend class Cache;
  int size;
  std::vector<bool> allocated;
  std::priority_queue<std::pair<int, int>> gaps; // (size, addr)
  RAM(int = 8);
  protected:
  std::vector<int> mem;
public:
  static RAM* createInstance(int);
  int getSize() const;
  int allocate(int);
  void deallocate(int, int = 1);
  void clear() {
    std::fill(mem.begin(), mem.end(), 0);
    gaps = std::priority_queue<std::pair<int, int>>(); // reset gaps
    gaps.push({size, 0});
    std::fill(allocated.begin(), allocated.end(), false);
  } // clear memory
};

#endif