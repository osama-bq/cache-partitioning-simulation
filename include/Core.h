#ifndef CORE_H
#define CORE_H

#include <thread>
#include "RAM.h"
#include "Cache.h"
#include "Instruction.h"
#include "Process.h"

class Core {
  static RAM* ram;
  static Cache* cache;
  int id, pc, dataAddr = -1;
  char acc;
  Instruction ir;
  bool busy = false;

  void runInstruction();
  void loadIR();
  void load(int);
  void store(int);
  void set(char);
public:
  Core(int);
  int getId() const;
  bool isBusy() const;
  std::thread runProcess(Process&);
};

#endif