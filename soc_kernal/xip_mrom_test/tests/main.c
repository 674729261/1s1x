#include <stdint.h>
#define XIP_BASE 0x30000000
void putch(char c);
uint32_t inl(uint32_t addr);

void _start() {
  asm volatile("lui sp, 0x20000");
  for (uint32_t i = 0; i < 0x80; i += 4) {
    uint32_t recv = inl(XIP_BASE + i);

    char buffer[16] = {};
    int cnt = 0;
    while (recv) {
      int dig = recv % 16;
      buffer[cnt++] = (dig < 10 ? '0' + dig : 'a' + dig - 10);
      recv /= 16;
    }
    for (int j = cnt - 1; j >= 0; j--)
      putch(buffer[j]);
    putch('\n');
  }

  asm volatile("lui t0, 0x30000");
  asm volatile("jalr x0, 0(t0)");
}

void putch(char c) { *(volatile char *)0x10000000L = c; }
uint32_t inl(uint32_t addr) { return *(volatile uint32_t *)addr; }