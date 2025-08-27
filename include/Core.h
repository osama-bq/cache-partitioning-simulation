#ifndef CORE_H
#define CORE_H

#include <thread>
#include "RAM.h"
#include "Instruction.h"
#include "Process.h"

class Core {
  static RAM* ram;
  int id, acc, pc, dataAddr;
  Instruction ir;
  bool busy = false;

  void runInstruction();
  void loadIR();
public:
  Core(int);
  int getId() const;
  bool isBusy() const;
  std::thread runProcess(Process&);
};

#endif