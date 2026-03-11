#include <klib.h>
#include <stdint.h>
int v = 0;
__attribute__((noinline)) void f() { asm volatile("ret"); }
__attribute__((noinline)) void g() { asm volatile("call h"); }
__attribute__((noinline)) void h() { asm volatile("call i"); }
__attribute__((noinline)) void i() { v++; }

int main() {
  uint32_t *p_f = (uint32_t *)f;
  uint32_t *p_g = (uint32_t *)g;
  asm volatile("call g");
  *(volatile uint32_t *)p_g = *p_f;
  g();
  printf("V is %d, should be 1", v);
  if (v == 1)
    return -1;
  else
    return 0;
}