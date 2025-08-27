#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include "OS.h"

using namespace std;

int main() {
  auto os = OS::createInstance();
  auto p1 = Process::createRandom(5, os->getRAM()->getSize());
  try {
    os->loadProcess(p1);
    os->loadProcess(p1);
    cout << "Process " << p1.getId() << " loaded successfully." << endl;
    for (auto& byte: os->getRAM()->mem) {
      cout << hex << (static_cast<int>(byte) & 0xFF) << " ";
    }
  } catch (const runtime_error& e) {
    cerr << "Error: " << e.what() << endl;
  }
  return 0;
}