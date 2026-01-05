#include "riscv/riscv.h"
#include "soc.h"
#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
static const char mainargs[MAINARGS_MAX_LEN] =
    TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

void putch(char ch) { outb(SERIAL_PORT, ch); }

void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : : "r"(code));
  while (1)
    ;
}

void _show_motd(uint32_t vendorid, uint32_t archid) {
  const char *first_part = "\033[31mmvendorid\033[0m : 0x";
  const char *second_part = "\n\033[31mmarchid\033[0m : ";
  char buffer[16] = {};
  int cnt = 0;
  for (const char *p = first_part; *p; p++)
    putch(*p);
  while (vendorid) {
    int dig = vendorid % 16;
    buffer[cnt++] = (dig < 10 ? '0' + dig : 'a' + dig - 10);
    vendorid /= 16;
  }
  for (int i = cnt - 1; i >= 0; i--)
    putch(buffer[i]);
  cnt = 0;
  for (const char *p = second_part; *p; p++)
    putch(*p);
  while (archid) {
    int dig = archid % 10;
    buffer[cnt++] = '0' + dig;
    archid /= 10;
  }
  for (int i = cnt - 1; i >= 0; i--)
    putch(buffer[i]);
  putch('\n');
}

void _trm_init() {
  // printf("\033[31mmvendorid\033[0m : %#010x\n\033[31mmarchid\033[0m : %d\n",
  //        vendorid, archid);

  extern char __data_load_start, __data_load_end, __data_start;
  char *src = &__data_load_start;
  char *dst = &__data_start;
  while (src < &__data_load_end) {
    *dst = *src;
    ++dst;
    ++src;
  }

  extern char __bss_start, __bss_end;
  for (char *p = &__bss_start; p < &__bss_end; p++)
    *p = 0;

  int ret = main(mainargs);
  halt(ret);
}
