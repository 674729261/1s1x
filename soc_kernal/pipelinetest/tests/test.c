#include <klib.h>
#include <stdint.h>

int main() {
  for (int i = 0; i < 65536; i++) {
    asm volatile("addi a1, a1, 1");
    asm volatile("add a1, a1, a1");
    asm volatile("addi a2, a2, 2");
    asm volatile("add a2, a2, a2");
    asm volatile("sub a1, a2, a1");
  }
}