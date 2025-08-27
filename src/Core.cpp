#include <stdexcept>
#include <unistd.h>
#include "Constants.h"
#include "Utilities.h"
#include "Core.h"
#include "Process.h"
#include "RAM.h"

#include <iostream>

RAM* Core::ram = nullptr;

Core::Core(int i) : id(i) {
  ram = RAM::getInstance();
}
int Core::getId() const { return id; }

bool Core::isBusy() const { return busy; }

void Core::runInstruction() {
  // const std::string ops[] = {"LOAD", "STORE", "EXECUTE"};
  // std::cout << (ops[static_cast<int>(ir.op)]) << " " << std::hex << ir.operand << std::dec << std::endl;
  switch (ir.op) {
    case Operator::LOAD:
      acc = ram->mem[dataAddr + ir.operand]; // Simplified for demonstration
      break;
    case Operator::STORE:
      ram->mem[dataAddr + ir.operand] = static_cast<char>(acc);
      break;
    case Operator::EXECUTE:
      usleep(100); // Simulate execution time
      break;
    default:
      throw std::runtime_error("Unknown instruction");
  }
}

void Core::loadIR() {
  ir.op = static_cast<Operator>(ram->mem[pc]);
  ir.operand = loadBytes(&ram->mem[pc + 1], SYSTEM_BITS / 8);
}

std::thread Core::runProcess(Process& p) {
  if (busy) throw std::runtime_error("Core " + std::to_string(id) + " is already busy.");
  if (p.getAddr() == -1) throw std::runtime_error("Process " + std::to_string(p.getId()) + " is not loaded in RAM.");
  return std::thread([this, &p](){
    busy = true;
    auto addr = p.getAddr();
    auto instrCount = p.instructionsCount();
    dataAddr = addr + instrCount * (1 + (SYSTEM_BITS / 8));
    pc = addr;
    acc = 0;
    // std::cout << "Core " << id << " executing process " << p.getId() << std::endl;
    for (int i = 0; i < instrCount; i++) {
      loadIR();
      runInstruction();
      pc += 1 + (SYSTEM_BITS / 8);
    }
    busy = false;
  });
}