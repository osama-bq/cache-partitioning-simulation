void storeBytes(char* addr, int value, int byteCount) {
  for (int i = 0; i < byteCount; i++, addr++, value >>= 8)
    *addr = static_cast<char>(value & 0xFF);
}