#include "RAM.h"
#include "Constants.h"
#include <cassert>

RAM::RAM(int numBlocks) : size(numBlocks * BLOCK_SIZE), mem(size), allocated(size) {
  gaps.push({size, 0});
}

int RAM::getSize() const { return size; }

RAM* RAM::createInstance(int s) {
  instance = std::unique_ptr<RAM>(new RAM(s));
  return instance.get();
}

int RAM::allocate(int size) {
  assert(size > 0 && "Size must be positive");
  auto [gap_size, addr] = gaps.top();

  if (gap_size < size)
    return -1; // avoiding rearrangement for simplicity
  gaps.pop();

  for (int i = addr; i < addr + size; i++)
    allocated[i] = true;

  if (gap_size > size)
    gaps.push({gap_size - size, addr + size});

  return addr;
}

void RAM::deallocate(int addr, int size) {
  assert(size > 0 && "Size must be positive");
  assert(addr >= 0 && addr + size <= this->size && "Address out of bounds");
  for (int i = addr; i < addr + size; i++)
    allocated[i] = false;
}