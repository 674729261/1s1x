#include <am.h>
#include <klib.h>
#include <soc.h>
#include <stdint.h>
#include <sys/types.h>

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint8_t signal = inb(KBD_ADDR);
  kbd->keycode = signal;
}
