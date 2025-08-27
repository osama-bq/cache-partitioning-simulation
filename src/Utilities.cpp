void storeBytes(char* addr, int value, int byteCount) {
  addr += byteCount - 1;
  for (int i = 0; i < byteCount; i++, addr--, value >>= 8)
    *addr = static_cast<char>(value & 0xFF);
}

int loadBytes(char* addr, int byteCount) {
  int value = 0;
  for (int i = 0; i < byteCount; i++, addr++)
    value = (value << 8) | static_cast<int>(*addr);
  return value;
}