#include "hw2_test.h"
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <csignal>

using namespace std;

int main() {
  cout << "calling get_all_cs" << endl;
  int x = get_all_cs();
	cout << "sum of all cs: " << x << endl;
  set_status(1);
  register_process();
  x = get_all_cs();
	cout << "sum of all cs after: " << x << endl;
  set_status(0);
  x = get_all_cs();
	cout << "sum of all cs after 2: " << x << endl;
  kill(getpid(), SIGSTOP);
  x = get_all_cs();
	cout << "sum of all cs after 3: " << x << endl;
  set_status(1);
  x = get_all_cs();
  cout << "sum of all cs after 4: " << x << endl;
  return 0;
}

