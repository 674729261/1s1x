#include "am.h"
#include <klib.h>
#include <stdint.h>

#define SWITCH_GPIO 0x10002004
#define DIGIT_GPIO 0x10002008

int main() {
  // uint16_t switches = *(volatile uint16_t *)SWITCH_GPIO;
  *(volatile uint32_t *)DIGIT_GPIO = 0xdeadbeef;
  *(volatile uint32_t *)(DIGIT_GPIO + 0x4) = 0x12345678;
  *(volatile uint16_t *)(DIGIT_GPIO + 0x2) = 0x3456;
  *(volatile uint8_t *)(DIGIT_GPIO + 0x5) = 0x89;

  return 0;
}
