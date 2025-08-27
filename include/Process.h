#ifndef PROCESS_H
#define PROCESS_H

#include <vector>
#include "Instruction.h"

class Process;

class Process {
  static int nextId;
  int id;
  public:
  std::vector<Instruction> instructions;
  Process();
  int getId() const;
  int size() const;

  static Process createRandom(int, int);
};

#endif