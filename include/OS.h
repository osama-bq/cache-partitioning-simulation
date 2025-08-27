#ifndef OS_H
#define OS_H

#include <memory>
#include "CPU.h"
#include "RAM.h"
#include "Cache.h"
#include "Process.h"

class OS {
  static std::unique_ptr<OS> os;
  CPU* cpu;
  RAM* ram;
  Cache* cache;
  OS(int cpu_cores = 2, int ram_size = 32, int cache_size = 4, int cache_ways = 2);
public:
  static OS* createInstance(int, int, int, int);
  static OS* createInstance();
  CPU* getCPU() const;
  RAM* getRAM() const;
  Cache* getCache() const;
  void loadProcess(Process&);
};

#endif