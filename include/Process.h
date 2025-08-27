#ifndef PROCESS_H
#define PROCESS_H

#include <vector>
#include "Instruction.h"

class Process;

class Process {
  friend class OS;
  static int nextId;
  int id, dataSpace;
protected:
  int addr = -1; // Address in RAM where process is loaded
  std::vector<Instruction> instructions;
public:
  Process(int);
  int getId() const;
  int size() const;
  int instructionsCount() const;
  int getAddr() const;
  static Process createRandom(int, int);
};

#endif