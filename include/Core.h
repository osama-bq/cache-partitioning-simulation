#ifndef CORE_H
#define CORE_H

#include "Process.h"

class Core {
  int id;
  bool busy = false;
public:
  Core(int);
  int getId() const;
  bool isBusy() const;
  void runProcess(Process&);
};

#endif