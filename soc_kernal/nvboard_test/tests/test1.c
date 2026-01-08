#include "am.h"
#include <klib.h>
#include <stdint.h>

#define LED_GPIO 0x10002000
#define SWITCH_GPIO 0x10002004
#define DIGIT_GPIO 0x10002008

const uint8_t hex_2_digit_ctrl[16] = {0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49,
                                      0x41, 0x1F, 0x01, 0x09, 0x11, 0xC1,
                                      0x63, 0x85, 0x61, 0x71};

int main() {
  uint32_t marchid;
  asm volatile("csrr %0, marchid" : "=r"(marchid));
  for (int i = 0; i < 8; i++) {
    *(volatile uint8_t *)(DIGIT_GPIO + i) = hex_2_digit_ctrl[marchid % 10];
    marchid /= 10;
  }

loop:
  for (uint16_t p = 0; p < 16; p++) {
    *(volatile uint16_t *)LED_GPIO = (1 << p);
    while (*(volatile uint16_t *)SWITCH_GPIO != 0x8001)
      ;
    uint8_t low = p & 0xff;
    uint8_t high = (p >> 8) & 0xff;
    putch(high < 10 ? '0' + high : 'A' + high - 10);
    putch(low < 10 ? '0' + low : 'A' + low - 10);
    putch('\n');
    for (volatile int j = 0; j < 1024; j++)
      ;
  }
  goto loop;
  return 0;
}
