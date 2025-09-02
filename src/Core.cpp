#include <stdexcept>
#include <unistd.h>
#include "Constants.h"
#include "Utilities.h"
#include "Core.h"
#include "Process.h"
#include "RAM.h"
#include "OS.h"

#include <iostream>

RAM* Core::ram = nullptr;
Cache* Core::cache = nullptr;

Core::Core(int i) : id(i) {
  ram = RAM::getInstance();
  cache = Cache::getInstance();
}
int Core::getId() const { return id; }

bool Core::isBusy() const { return busy; }

void Core::load(int addr) {
    if (addr < 0 || addr >= ram->getSize())
        throw std::runtime_error("Invalid memory access at " + std::to_string(addr));

    acc = cache->get(addr, id);   // cache decides hit/miss
}

void Core::store(int addr) {
    if (addr < 0 || addr >= ram->getSize())
        throw std::runtime_error("Invalid memory access at " + std::to_string(addr));

    cache->set(addr, acc, id);    // cache decides hit/miss
}


void Core::set(char val) { acc = val; }

void Core::runInstruction() {
  // const std::string ops[] = {"LOAD", "STORE", "EXECUTE"};
  // std::cout << (ops[static_cast<int>(ir.op)]) << " " << std::hex << ir.operand << std::dec << std::endl;
  switch (ir.op) {
    case Operator::LOAD:
      load(dataAddr + ir.operand); // Simplified for demonstration
      break;
    case Operator::STORE:
      store(dataAddr + ir.operand);
      break;
    case Operator::SET:
      set(ir.operand);
      break;
    case Operator::EXECUTE:
      usleep(100); // Simulate execution time
      break;
    default:
      throw std::runtime_error("Unknown instruction" + std::to_string(static_cast<int>(ir.op)));
  }
}

void Core::loadIR() {
  ir.op = static_cast<Operator>(ram->mem[pc++]);
  ir.operand = ram->mem[pc++];
}

std::thread Core::runProcess(Process& p) {
  if (busy) throw std::runtime_error("Core " + std::to_string(id) + " is already busy.");
  if (p.getAddr() == -1) throw std::runtime_error("Process " + std::to_string(p.getId()) + " is not loaded in RAM.");

  dataAddr = p.getAddr() + p.instructionsCount() * 2;

  // Capture a copy of the process to avoid reference lifetime issues.
  Process procCopy = p;

  return std::thread([this, procCopy]() mutable {
    busy = true;
    auto addr = procCopy.getAddr();
    auto instrCount = procCopy.instructionsCount();
    pc = addr;
    acc = 0;
    for (int i = 0; i < instrCount; i++) {
      loadIR();
      runInstruction();
    }
    busy = false;
  });
}