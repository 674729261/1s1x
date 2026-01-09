#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <soc.h>
#include <stdint.h>
#include <sys/types.h>

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  enum { ST_FRESH, ST_E0 };
  static uint8_t state = ST_FRESH;
  static bool is_keydown = true;
  static uint8_t discard_cnt = 0;
  uint8_t signal = inb(KBD_ADDR);
  if (signal == 0) {
    kbd->keycode = 0;
    kbd->keydown = false;
    return;
  }
  if (discard_cnt != 0) {
    discard_cnt--;
    kbd->keycode = 0;
    kbd->keydown = false;
    return;
  }
  if (signal == 0xf0) {
    kbd->keycode = 0;
    kbd->keydown = false;
    is_keydown = false;
    return;
  }
  switch (state) {
  case ST_FRESH:
    if (signal == 0xe0) {
      state = ST_E0;
      kbd->keycode = 0;
      kbd->keydown = false;
      return;
    } else if (signal == 0xe1) {
      discard_cnt = 7;
      kbd->keycode = (0xe100) | signal;
      kbd->keydown = is_keydown;
      is_keydown = true;
      return;
    } else {
      kbd->keycode = signal;
      kbd->keydown = is_keydown;
      is_keydown = true;
      return;
    }
    break;
  case ST_E0:
  default:
    state = ST_FRESH;
    if (signal == 0x12) {
      // discard_cnt = 2;
      kbd->keycode = AM_KEY_PRTSCR;
      kbd->keydown = is_keydown;
      is_keydown = true;
      return;
    } else {
      kbd->keycode = (0xe000) | signal;
      kbd->keydown = is_keydown;
      is_keydown = true;
      return;
    }
    break;
  }
}
