#include <klib.h>
#include <stdint.h>

int main() {
  for (int i = 0; i < 32768; i++) {
    asm volatile("addi a1, a1, 1");
    asm volatile("add a1, a1, a1");
  }
}