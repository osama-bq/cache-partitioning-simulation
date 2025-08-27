#ifndef RAM_H
#define RAM_H

#include <vector>
#include <queue>
#include "HardwareComponent.h"

class RAM : public HardwareComponent<RAM> {
  friend class HardwareComponent<RAM>;
  int size;
  std::vector<bool> allocated;
  std::priority_queue<std::pair<int, int>> gaps; // (size, addr)
  RAM(int s = 32);
public:
  std::vector<char> mem;
  static RAM* createInstance(int);
  int getSize() const;
  int allocate(int);
  void deallocate(int, int = 1);
};

#endif