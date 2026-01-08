#include "am.h"
#include <klib.h>
#include <stdint.h>

#define SWITCH_GPIO 0x10002004
#define DIGIT_GPIO 0x10002008

int main() {
  uint16_t switches = *(volatile uint16_t *)SWITCH_GPIO;
  *(volatile uint16_t *)DIGIT_GPIO = switches;

  return 0;
}
