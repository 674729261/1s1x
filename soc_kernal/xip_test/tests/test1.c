#include <klib.h>
#include <spi.h>
#include <stdint.h>

#define MY_CHECK(C)                                                            \
  do {                                                                         \
    if (!(C))                                                                  \
      return -1;                                                               \
  } while (0)
#define XIP_BASE 0x3007f0f0
int main() {
  uint32_t recv = inl(XIP_BASE);
  char buffer[16] = {};
  int cnt = 0;
  MY_CHECK(recv == 0x7f0f0);
  while (recv) {
    int dig = recv % 16;
    buffer[cnt++] = (dig < 10 ? '0' + dig : 'a' + dig - 10);
    recv /= 16;
  }
  for (int i = cnt - 1; i >= 0; i--)
    putch(buffer[i]);
  putch('\n');
}
