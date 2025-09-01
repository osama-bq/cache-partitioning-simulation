#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include "OS.h"

using namespace std;

int main() {
  auto os = OS::createInstance(4, 16384, 128, 8);
  vector<Process> processes;
  vector<thread> t;
  for (int i = 0; i < 4; i++) {
    processes.push_back(Process::createRandom(350, 100));
    os->loadProcess(processes.back());
    cout << "Loaded process " << processes.back().getId() << " at address "
         << processes.back().getAddr() << " (size: " << processes.back().size() << ")\n";
  }

  auto cpu = os->getCPU();
  auto& cores = cpu->getCores();
  for (int i = 0; i < processes.size(); i++)
    t.push_back(cores[i].runProcess(processes[i]));

  for (auto& th : t) th.join();
  return 0;
}