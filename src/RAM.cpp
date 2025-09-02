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

int RAM::allocate(int requestSize) {
    assert(requestSize > 0 && "Size must be positive");

    // Round up to full blocks
    int blocksNeeded = (requestSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int allocSize = blocksNeeded * BLOCK_SIZE;

    auto [gap_size, addr] = gaps.top();

    if (gap_size < allocSize)
        return -1; // Not enough space in this gap, could try other gaps if implemented
    gaps.pop();

    // Mark all bytes in the allocated blocks as used
    for (int i = addr; i < addr + allocSize; i++)
        allocated[i] = true;

    // If leftover space in the gap, push it back
    if (gap_size > allocSize)
        gaps.push({gap_size - allocSize, addr + allocSize});

    return addr;
}

void RAM::deallocate(int addr, int size) {
  assert(size > 0 && "Size must be positive");
  assert(addr >= 0 && addr + size <= this->size && "Address out of bounds");
  for (int i = addr; i < addr + size; i++)
    allocated[i] = false;
}