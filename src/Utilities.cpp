#include <vector>
#include "Utilities.h"
#include "Constants.h"

void storeBytes(std::vector<int>::iterator it, int value, int byteCount) {
  it += byteCount - 1;
  for (int i = 0; i < byteCount; i++, it--, value >>= 8)
    *it = static_cast<char>(value & 0xFF);
}

int loadBytes(std::vector<int>::const_iterator it, int byteCount) {
  int value = 0;
  for (int i = 0; i < byteCount; i++, it++)
    value = (value << 8) | static_cast<int>(*it);
  return value;
}