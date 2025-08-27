#include "OS.h"
#include "Constants.h"
#include "Utilities.h"

OS::OS(int cpu_cores, int ram_size, int cache_size, int cache_ways) {
  cpu = CPU::createInstance(cpu_cores);
  ram = RAM::createInstance(ram_size);
  cache = Cache::createInstance(cache_size, cache_ways);
}

OS* OS::createInstance(int cpu_cores, int ram_size, int cache_size, int cache_ways) {
  os = std::unique_ptr<OS>(new OS(cpu_cores, ram_size, cache_size, cache_ways));
  return os.get();
}

OS* OS::createInstance() {
  os = std::unique_ptr<OS>(new OS());
  return os.get();
}

CPU* OS::getCPU() const {
  return cpu;
}

RAM* OS::getRAM() const {
  return ram;
}

Cache* OS::getCache() const {
  return cache;
}

void OS::loadProcess(Process& p) {
  auto addr = ram->allocate(p.size());
  if (addr == -1)
    throw std::runtime_error("Not enough RAM to load process " + std::to_string(p.getId()));
  for (int i = 0; i < p.instructions.size(); i++) {
    ram->mem[addr + i * (1 + (SYSTEM_BITS / 8))] = static_cast<char>(p.instructions[i].op);
    storeBytes(&ram->mem[addr + i * (1 + (SYSTEM_BITS / 8)) + 1], p.instructions[i].operand, SYSTEM_BITS / 8);
  }
}

std::unique_ptr<OS> OS::os = nullptr;