#include <klib.h>
#include <stdint.h>

#define SPI_BASE 0x10001000

#define MY_CHECK(C)                                                            \
  do {                                                                         \
    if (!(C))                                                                  \
      return -1;                                                               \
  } while (0)

int main() {
  uint32_t ctrl = *(volatile uint32_t *)(SPI_BASE + 0x10);
  while (ctrl & (1 << 8))
    ctrl = *(volatile uint32_t *)(SPI_BASE + 0x10);
  ctrl |= (1 << 9);
  ctrl &= ~0x7f;
  ctrl |= 16;
  *(volatile uint32_t *)(SPI_BASE + 0x10) = ctrl;
  *(volatile uint32_t *)(SPI_BASE + 0x14) = 0x000000ff;
  *(volatile uint32_t *)(SPI_BASE + 0x18) = 7;
  while (ctrl & (1 << 8))
    ctrl = *(volatile uint32_t *)(SPI_BASE + 0x10);
  *(volatile uint32_t *)(SPI_BASE) = 0x2c;
  ctrl |= (1 << 8);
  *(volatile uint32_t *)(SPI_BASE + 0x10) = ctrl;
  uint32_t recv = *(volatile uint32_t *)(SPI_BASE);

  char buffer[16] = {};
  int cnt = 0;
  while (recv) {
    int dig = recv % 16;
    buffer[cnt++] = (dig < 10 ? '0' + dig : 'a' + dig - 10);
    recv /= 16;
  }
  for (int i = cnt - 1; i >= 0; i--)
    putch(buffer[i]);
  return 0;
}
