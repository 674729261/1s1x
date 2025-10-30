#include <Device/Device.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <atomic>
#include <cstdint>
#include <print>
#include <spdlog/spdlog.h>

#define MAP(c, f) c(f)
#define NEMU_KEYS(f)                                                           \
  f(ESCAPE) f(F1) f(F2) f(F3) f(F4) f(F5) f(F6) f(F7) f(F8) f(F9) f(F10)       \
      f(F11) f(F12) f(GRAVE) f(1) f(2) f(3) f(4) f(5) f(6) f(7) f(8) f(9) f(0) \
          f(MINUS) f(EQUALS) f(BACKSPACE) f(TAB) f(Q) f(W) f(E) f(R) f(T) f(Y) \
              f(U) f(I) f(O) f(P) f(LEFTBRACKET) f(RIGHTBRACKET) f(BACKSLASH)  \
                  f(CAPSLOCK) f(A) f(S) f(D) f(F) f(G) f(H) f(J) f(K) f(L)     \
                      f(SEMICOLON) f(APOSTROPHE) f(RETURN) f(LSHIFT) f(Z) f(X) \
                          f(C) f(V) f(B) f(N) f(M) f(COMMA) f(PERIOD) f(SLASH) \
                              f(RSHIFT) f(LCTRL) f(APPLICATION) f(LALT)        \
                                  f(SPACE) f(RALT) f(RCTRL) f(UP) f(DOWN)      \
                                      f(LEFT) f(RIGHT) f(INSERT) f(DELETE)     \
                                          f(HOME) f(END) f(PAGEUP) f(PAGEDOWN)

#define NEMU_KEY_NAME(k) NEMU_KEY_##k,

enum { NEMU_KEY_NONE = 0, MAP(NEMU_KEYS, NEMU_KEY_NAME) };

#define SDL_KEYMAP(k) keymap[SDL_SCANCODE_##k] = NEMU_KEY_##k;
static uint32_t keymap[256] = {};

void Devices::init_keymap() { MAP(NEMU_KEYS, SDL_KEYMAP) }

void Devices::init_keyboard() { init_keymap(); }

uint32_t wrap_key_event(uint8_t scancode, bool is_keydown) {
  return keymap[scancode] | (is_keydown ? 0x8000 : 0);
}

void Devices::process_keyboard() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      quit.store(true);
      break;
    }

    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      uint8_t k = event.key.keysym.scancode;
      bool is_keydown = (event.key.type == SDL_KEYDOWN);
      uint32_t wrapped = wrap_key_event(k, is_keydown);

      if (!key_queue->Push(wrapped))
        spdlog::warn("Key event {:#08x} ignored because the buffer is full",
                     wrapped);

      //   spdlog::info("Key event {:#08x} detected", wrapped);
      break;
    }
    }
  }
}