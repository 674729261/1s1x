#include "am.h"
#include <klib.h>
#include <stdint.h>

#define LED_GPIO 0x10002000
#define SWITCH_GPIO 0x10002004
#define DIGIT_GPIO 0x10002008

int main() {
  for (int i = 0; i < 100; i++) {
    for (uint16_t p = 0; p < 16; p++) {
      *(volatile uint16_t *)LED_GPIO = (1 << p);
      for (volatile int j = 0; j < 256; j++)
        ;
    }
  }
  return 0;
}
