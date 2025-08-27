#include "CPU.h"
#include "Core.h"

CPU::CPU(int n) : num_cores(n) {
  for (int i = 0; i < n; i++)
    cores.emplace_back(Core(i));
}

CPU* CPU::createInstance(int n) {
  instance = std::unique_ptr<CPU>(new CPU(n));
  return instance.get();
}

std::vector<Core>& CPU::getCores() { return cores; }