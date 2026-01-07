#include "riscv/riscv.h"
#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <spi.h>
#include <stdint.h>

extern char _heap_start;
int main(const char *args);

// extern char _pmem_start;
// #define PMEM_SIZE (128 * 1024 * 1024)
// #define PMEM_END ((uintptr_t)&_pmem_start + PMEM_SIZE)

extern char _heap_end;
Area heap = RANGE(&_heap_start, &_heap_end);
static const char mainargs[MAINARGS_MAX_LEN] =
    TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

void putch(char ch) {
  uint8_t line_status = 0x00;
  while (!(line_status & (1 << 5)))
    line_status = inb(SERIAL_PORT + SERIAL_LSR_OFFSET);

  outb(SERIAL_PORT, ch);
}

void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : : "r"(code));
  while (1)
    ;
}

void _init_uart() {
  uint8_t lcr = inb(SERIAL_PORT + SERIAL_LCR_OFFSET);
  lcr |= (1 << 7);
  outb(SERIAL_PORT + SERIAL_LCR_OFFSET, lcr);

  outb(SERIAL_PORT + 1, 0x00);
  outb(SERIAL_PORT, 0x01);

  lcr = inb(SERIAL_PORT + SERIAL_LCR_OFFSET);
  lcr &= ~(1 << 7);
  outb(SERIAL_PORT + SERIAL_LCR_OFFSET, lcr);
  outb(SERIAL_PORT + SERIAL_INT_OFFSET, 0x00);
}
void _show_motd(uint32_t vendorid, uint32_t archid) {
  const char message[] = "Second-stage Bootloader finished\n";
  for (const char *p = message; *p; p++)
    putch(*p);

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
  // wait_spi_finish();
  // spi_init();

  int ret = main(mainargs);
  halt(ret);
}

__attribute__((section(".fsbl"))) void fstbootloader() {
  extern char __ssbl_load_start, __ssbl_load_end, __ssbl_start;
  char *src = &__ssbl_load_start;
  char *dst = &__ssbl_start;
  while (src < &__ssbl_load_end) {
    *dst = *src;
    ++dst;
    ++src;
  }
}

__attribute__((section(".ssbl"))) static void *
__ssbl__memcpy(void *out, const void *in, size_t n) {
  unsigned char *d = (unsigned char *)out;
  const unsigned char *s = (const unsigned char *)in;

  while ((((uintptr_t)d) & 0x3) && n) {
    *d++ = *s++;
    n--;
  }

  while (n >= 4) {
    *(uint32_t *)d = *(const uint32_t *)s;
    d += 4;
    s += 4;
    n -= 4;
  }

  while (n-- > 0) {
    *d++ = *s++;
  }
  return out;
}

__attribute__((section(".ssbl"))) static void *__ssbl_memset(void *s, int c,
                                                             size_t n) {
  unsigned char *p = (unsigned char *)s;
  while ((((uintptr_t)p) & 0x3) && n) {
    *p++ = (unsigned char)c;
    n--;
  }
  const uint32_t content = ((unsigned char)c << 24) | ((unsigned char)c << 16) |
                           ((unsigned char)c << 8) | (unsigned char)c;
  while (n >= 4) {
    *(uint32_t *)p = content;
    p += 4;
    n -= 4;
  }

  const unsigned char uc = (unsigned char)c;
  while (n-- > 0) {
    *p++ = uc;
  }
  return s;
}

__attribute__((section(".ssbl"))) void secbootloader() {
  extern char __data_load_start, __data_load_end, __data_start;
  char *src = &__data_load_start;
  char *dst = &__data_start;
  __ssbl__memcpy(dst, src, &__data_load_end - &__data_load_start);

  extern char __prog_load_start, __prog_load_end, __prog_start;
  src = &__prog_load_start;
  dst = &__prog_start;
  __ssbl__memcpy(dst, src, &__prog_load_end - &__prog_load_start);

#ifdef RT_THREAD_LOAD
  extern char rt_load_begin, rt_load_end, rt_begin;
  src = &rt_load_begin;
  dst = &rt_begin;
  __ssbl__memcpy(dst, src, &rt_load_end - &rt_load_begin);
#endif

  extern char __bss_start, __bss_end;
  __ssbl_memset(&__bss_start, 0, &__bss_end - &__bss_start);
}