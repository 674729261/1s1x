#include "am.h"
#include <klib.h>
#include <stdint.h>

#define LED_GPIO 0x10002000
#define SWITCH_GPIO 0x10002004
#define DIGIT_GPIO 0x10002008

const uint8_t hex_2_digit[16] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d,
                                 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c,
                                 0x39, 0x5e, 0x79, 0x71};

int main() {
  uint32_t marchid;
  asm volatile("csrr %0, marchid" : "=r"(marchid));
  for (int i = 0; i < 8; i++) {
    *(volatile uint8_t *)(DIGIT_GPIO + i) = ~hex_2_digit[marchid % 10];
    marchid /= 10;
  }

loop:
  for (uint16_t p = 0; p < 16; p++) {
    *(volatile uint16_t *)LED_GPIO = (1 << p);
    while (*(volatile uint16_t *)SWITCH_GPIO != 0x8001)
      ;
    for (volatile int j = 0; j < 256; j++)
      ;
  }
  goto loop;
  return 0;
}
