#ifndef INSTRUCTION_H
#define INSTRUCTION_H

enum class Operator { LOAD, STORE, EXECUTE };

struct Instruction {
  Operator op;
  int operand;
  Instruction() = default;
  Instruction(Operator, int);
};

#endif