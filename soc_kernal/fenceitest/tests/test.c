#include <klib.h>
#include <stdint.h>
int v = 0;
__attribute__((noinline)) __attribute__((aligned(32))) int f() {
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  return 0;
}
__attribute__((noinline)) void g() { asm volatile("li a0, 456"); }

int main() {
  f();
  volatile uint32_t *p = (uint32_t *)f;
  for (int i = 0; i < 16; i++) {
    *p = *(uint32_t *)g;
    p++;
  }
  asm volatile("fence.i");
  volatile int r = f();
  printf("r is %d, should be 456\n", r);
  if (r == 456)
    return 0;
  else
    return -1;
}