#include <klib.h>
#include <stdint.h>
int v = 0;
__attribute__((noinline)) void f() { asm volatile("ret"); }

__attribute__((noinline)) void g() { asm volatile("call h"); }
__attribute__((noinline)) void h() { v++; }

int main() {
  uint32_t *p_f = (uint32_t *)f;
  uint32_t *p_g = (uint32_t *)g;
  asm volatile("call g");
  *(volatile uint32_t *)p_g = *p_f;
  g();
}