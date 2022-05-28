#include "hw2_test.h"

long hello(void) {
    return syscall(333);
}

long set_status(int status) {
    return syscall(334, status);
}

int get_status() {
    return (int)syscall(335);
}

int register_process() {
	return (int)syscall(336);
}

long get_all_cs() {
	return syscall(337);
}