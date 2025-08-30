#ifndef CPU_H
#define CPU_H

#include <vector>
#include <memory>
#include "HardwareComponent.h"
#include "Core.h"

class CPU : public HardwareComponent<CPU> {
  friend class HardwareComponent<CPU>;
  int num_cores;
  std::vector<Core> cores;
  CPU(int n = 2);
public:
  static CPU* createInstance(int);
  int getNumCores() const;
  std::vector<Core>& getCores();
};

#endif