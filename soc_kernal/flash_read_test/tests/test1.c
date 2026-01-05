#include <klib.h>
#include <spi.h>
#include <stdint.h>
#define SPI_BASE 0x10001000
#define MY_CHECK(C)                                                            \
  do {                                                                         \
    if (!(C))                                                                  \
      return -1;                                                               \
  } while (0)

uint32_t flash_read(uint32_t *addr) { return 0; }
int main() {}
