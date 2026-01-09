#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <soc.h>
#include <stdint.h>
#include <sys/types.h>

/* PS/2 scancode (set 1) -> AM key mapping (single-byte makes) */
static const int ps2_scancode_to_am[256] = {
    [0x01] = AM_KEY_ESCAPE,
    [0x02] = AM_KEY_1,
    [0x03] = AM_KEY_2,
    [0x04] = AM_KEY_3,
    [0x05] = AM_KEY_4,
    [0x06] = AM_KEY_5,
    [0x07] = AM_KEY_6,
    [0x08] = AM_KEY_7,
    [0x09] = AM_KEY_8,
    [0x0A] = AM_KEY_9,
    [0x0B] = AM_KEY_0,
    [0x0C] = AM_KEY_MINUS,
    [0x0D] = AM_KEY_EQUALS,
    [0x0E] = AM_KEY_BACKSPACE,
    [0x0F] = AM_KEY_TAB,
    [0x10] = AM_KEY_Q,
    [0x11] = AM_KEY_W,
    [0x12] = AM_KEY_E,
    [0x13] = AM_KEY_R,
    [0x14] = AM_KEY_T,
    [0x15] = AM_KEY_Y,
    [0x16] = AM_KEY_U,
    [0x17] = AM_KEY_I,
    [0x18] = AM_KEY_O,
    [0x19] = AM_KEY_P,
    [0x1A] = AM_KEY_LEFTBRACKET,
    [0x1B] = AM_KEY_RIGHTBRACKET,
    [0x1C] = AM_KEY_RETURN,
    [0x1D] = AM_KEY_LCTRL,
    [0x1E] = AM_KEY_A,
    [0x1F] = AM_KEY_S,
    [0x20] = AM_KEY_D,
    [0x21] = AM_KEY_F,
    [0x22] = AM_KEY_G,
    [0x23] = AM_KEY_H,
    [0x24] = AM_KEY_J,
    [0x25] = AM_KEY_K,
    [0x26] = AM_KEY_L,
    [0x27] = AM_KEY_SEMICOLON,
    [0x28] = AM_KEY_APOSTROPHE,
    [0x29] = AM_KEY_GRAVE,
    [0x2A] = AM_KEY_LSHIFT,
    [0x2B] = AM_KEY_BACKSLASH,
    [0x2C] = AM_KEY_Z,
    [0x2D] = AM_KEY_X,
    [0x2E] = AM_KEY_C,
    [0x2F] = AM_KEY_V,
    [0x30] = AM_KEY_B,
    [0x31] = AM_KEY_N,
    [0x32] = AM_KEY_M,
    [0x33] = AM_KEY_COMMA,
    [0x34] = AM_KEY_PERIOD,
    [0x35] = AM_KEY_SLASH,
    [0x36] = AM_KEY_RSHIFT,
    [0x38] = AM_KEY_LALT,
    [0x39] = AM_KEY_SPACE,
    [0x3A] = AM_KEY_CAPSLOCK,
    [0x3B] = AM_KEY_F1,
    [0x3C] = AM_KEY_F2,
    [0x3D] = AM_KEY_F3,
    [0x3E] = AM_KEY_F4,
    [0x3F] = AM_KEY_F5,
    [0x40] = AM_KEY_F6,
    [0x41] = AM_KEY_F7,
    [0x42] = AM_KEY_F8,
    [0x43] = AM_KEY_F9,
    [0x44] = AM_KEY_F10,
    [0x57] = AM_KEY_F11,
    [0x58] = AM_KEY_F12,
};

/* Extended PS/2 scancode (E0 prefix) -> AM key mapping */
static const int ps2_scancode_e0_to_am[256] = {
    [0x1C] = AM_KEY_RETURN,                             /* keypad Enter */
    [0x1D] = AM_KEY_RCTRL,       [0x35] = AM_KEY_SLASH, /* keypad slash */
    [0x38] = AM_KEY_RALT,        [0x47] = AM_KEY_HOME,
    [0x48] = AM_KEY_UP,          [0x49] = AM_KEY_PAGEUP,
    [0x4B] = AM_KEY_LEFT,        [0x4D] = AM_KEY_RIGHT,
    [0x4F] = AM_KEY_END,         [0x50] = AM_KEY_DOWN,
    [0x51] = AM_KEY_PAGEDOWN,    [0x52] = AM_KEY_INSERT,
    [0x53] = AM_KEY_DELETE,      [0x12] = AM_KEY_PRTSCR, /* partial support */
    [0x5B] = AM_KEY_APPLICATION, [0x5C] = AM_KEY_APPLICATION,
    [0x5D] = AM_KEY_APPLICATION,
};

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
      /* pause/break (multi-byte) - leave as before */
      discard_cnt = 7;
      kbd->keycode = (0xe100) | signal;
      kbd->keydown = is_keydown;
      is_keydown = true;
      return;
    } else {
      int code = ps2_scancode_to_am[signal];
      if (code != AM_KEY_NONE) {
        kbd->keycode = code;
        kbd->keydown = is_keydown;
      } else {
        kbd->keycode = 0;
        kbd->keydown = false;
      }
      is_keydown = true;
      return;
    }
    break;
  case ST_E0:
  default:
    state = ST_FRESH;
    {
      int code = ps2_scancode_e0_to_am[signal];
      if (code != AM_KEY_NONE) {
        kbd->keycode = code;
        kbd->keydown = is_keydown;
      } else {
        kbd->keycode = 0;
        kbd->keydown = false;
      }
      is_keydown = true;
      return;
    }
    break;
  }
}
