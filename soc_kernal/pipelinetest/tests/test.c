#include <klib.h>
#include <stdint.h>

int main() {

  asm volatile("lui a0, 0xf\n"
               "begin_test_loop:\n"
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
               "addi a0, a0, -1\n"
               "bnez a0, begin_test_loop\n"
               "ret");
}