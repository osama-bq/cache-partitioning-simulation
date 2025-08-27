#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include "OS.h"

using namespace std;

int main() {
  auto os = OS::createInstance(2, 128, 8, 2);
  auto p1 = Process::createRandom(10, 4);
  try {
    os->loadProcess(p1);
    cout << "Process " << p1.getId() << " loaded successfully at " << hex << p1.getAddr() << dec << endl;
    auto& cores = os->getCPU()->getCores();
    vector<thread> threads;
    for (auto& core: cores) {
      if (!core.isBusy()) {
        threads.push_back(core.runProcess(p1));
        break;
      }
    }
    for (auto& t: threads) t.join();
  } catch (const runtime_error& e) {
    cerr << "Error: " << e.what() << endl;
  }
  return 0;
}