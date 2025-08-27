#include "Core.h"
#include "Process.h"

Core::Core(int i) : id(i) {}
int Core::getId() const { return id; }

bool Core::isBusy() const { return busy; }

void Core::runProcess(Process& p) {
  busy = true;
  // Simulate process running
  busy = false;
}