#include <klib.h>
#include <stdint.h>

int main() {
  for (int i = 0; i < 65536; i++) {
    asm volatile("addi t0, t0, 1");
    asm volatile("add t0, t0, t0");
    asm volatile("addi t1, t1, 2");
    asm volatile("add t1, t1, t1");
    asm volatile("sub t0, t1, t0");
  }
}