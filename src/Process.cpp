#include "Process.h"
#include "RAM.h"
#include "Constants.h"

int Process::nextId = 1;

Process::Process() : id(nextId++) {}
int Process::getId() const { return id; }

int Process::size() const {
  return instructions.size() * (1 + (SYSTEM_BITS / 8));
}

Process Process::createRandom(int numInstructions, int maxOperand) {
  Process p;
  for (int i = 0; i < numInstructions; i++) {
    Operator op = static_cast<Operator>(rand() % 3);
    int operand = (op == Operator::EXECUTE) ? -1 : (rand() % maxOperand);
    p.instructions.emplace_back(op, operand);
  }
  return p;
}