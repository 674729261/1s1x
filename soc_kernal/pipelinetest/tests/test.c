#include <klib.h>
#include <stdint.h>

int main() {

  asm volatile("begin:\n"
               "addi t0, t0, 1\n"
               "add t0, t0, t0\n"
               "addi t1, t1, 2\n"
               "add t1, t1, t1\n"
               "sub t0, t1, t0\n"
               "addi t0, t0, 1\n"
               "add t0, t0, t0\n"
               "addi t1, t1, 2\n"
               "add t1, t1, t1\n"
               "sub t0, t1, t0\n"
               "j begin");
}