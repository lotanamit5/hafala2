#include "hw2_test.h"
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <csignal>
#include <sys/wait.h>

using namespace std;

int main() {

  cout << "PARENT PID = " << getpid() << endl;
  cout << "PARENT GAC 1 = " << get_all_cs() << endl;
  register_process();
  cout << "PARENT GAC 2 = " << get_all_cs() << endl;
  set_status(1);

  pid_t pid = fork();
  if (pid == 0) {
    // in child process
    cout << "CHILD PID = " << getpid() << endl;
    set_status(0);
    cout << "CHILD GAC 1 = " << get_all_cs() << endl;
    register_process();
    cout << "CHILD GAC 2 = " << get_all_cs() << endl;
    set_status(1);
    cout << "CHILD GAC 3 = " << get_all_cs() << endl;
    return 0;
  } else {
    sleep(1);
    cout << "PARENT GAC 3 = " << get_all_cs() << endl;
    wait(NULL);
    cout << "PARENT GAC 4 = " << get_all_cs() << endl;
  }

  return 0;
}