#include <klib.h>
#include <stdint.h>
int v = 0;
__attribute__((noinline)) int f() {
  asm volatile("li a0, 123");
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
  asm volatile("ret");
  return 0;
}
__attribute__((noinline)) void g() { asm volatile("li a0, 456"); }

int main() {
  f();
  volatile uint32_t *p = (uint32_t *)f;
  for (int i = 0; i < 4; i++) {
    *p = *(uint32_t *)g;
    p++;
  }
  int r = f();
  printf("r is %d, should be 456", r);
}