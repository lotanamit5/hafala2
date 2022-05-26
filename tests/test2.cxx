#include "hw2_test.h"
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <csignal>

using namespace std;

int main() {
  register_process();
  set_status(1);
  cout << "status for test2 set to 1" << endl;
  kill(getpid(), SIGSTOP);
  return 0;
}

