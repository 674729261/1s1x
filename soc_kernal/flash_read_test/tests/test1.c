#include "am.h"
#include <klib.h>
#include <stdint.h>

#define FLASH_BASE 0x30000000
#define MY_CHECK(C)                                                            \
  do {                                                                         \
    if (!(C))                                                                  \
      return -1;                                                               \
  } while (0)

int main() {
  for (int i = 0; i < 0x1000; i += 4) {
    uint32_t read_data = *(volatile uint32_t *)(FLASH_BASE + i);
    MY_CHECK(i == read_data + 1);
  }
}
