#include "Process.h"
#include "RAM.h"
#include "Constants.h"

#include <random>
#include <chrono>

// mersenne twister random number generator
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

int Process::nextId = 1;

Process::Process(int n) : dataSpace(n), id(nextId++) {}
int Process::getId() const { return id; }

int Process::size() const {
  return instructions.size() * (1 + (SYSTEM_BITS / 8)) + dataSpace;
}

int Process::getAddr() const { return addr; }

int Process::instructionsCount() const { return instructions.size(); }

Process Process::createRandom(int numInstructions, int dataSpace) {
  std::uniform_int_distribution<int> rndOp(0, 2), rndAddr(0, dataSpace - 1);
  Process p(dataSpace);
  for (int i = 0; i < numInstructions; i++) {
    Operator op = static_cast<Operator>(rndOp(rng) % 3);
    int operand = (op == Operator::EXECUTE) ? -1 : (rndAddr(rng) % dataSpace);
    p.instructions.emplace_back(op, operand);
  }
  return p;
}