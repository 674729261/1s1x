#pragma once

#include "Device/Audio.h"
#include "Device/RTC.h"
#include "lockfree/spsc/ring_buf.hpp"
#include "spdlog/spdlog.h"
#include <Device/VGA.h>
#include <SDL2/SDL.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <lockfree/lockfree.hpp>
#include <memory>
#include <thread>
constexpr uint32_t SERIAL_OFFSET = 0x00003f8;
constexpr uint32_t RTC_OFFSET = 0x0000048;
constexpr std::size_t RTC_LEN = sizeof(RTC.RTC_reg);
constexpr uint32_t AUDIO_CTL_OFFSET = 0x0000200;
constexpr std::size_t AUDIO_CTL_LEN = sizeof(Audio.reg_ctl);
constexpr uint32_t AUDIO_BF_OFFSET = 0x1200000;
constexpr std::size_t AUDIO_BF_LEN = AudioBase_t::SoundBufferSize;
constexpr uint32_t VGA_CTL_OFFSET = 0x0000100;
constexpr std::size_t VGA_CTL_LEN = 8;
constexpr uint32_t VGA_BF_OFFSET = 0x1000000;
constexpr std::size_t VGA_BF_LEN = VideoBase_t::VMemSize;

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
inline uint32_t keymap[256] = {};

inline void init_keymap() { MAP(NEMU_KEYS, SDL_KEYMAP) }

inline uint32_t wrap_key_event(uint8_t scancode, bool is_keydown) {
  return keymap[scancode] | (is_keydown ? 0x8000 : 0);
}

inline std::unique_ptr<std::jthread> device_thread;
inline std::atomic_bool quit;
void device_thread_work(std::promise<void> &device_inited_promise);

using KBD_BUF = lockfree::spsc::Queue<uint32_t, 128>;
inline std::unique_ptr<KBD_BUF> kbd_buf;

inline void init_vga_kbd() {
  Video.vmem1 = std::make_unique<VideoBase_t::VMEM>();
  Video.vmem2 = std::make_unique<VideoBase_t::VMEM>();

  spdlog::info("Allocated 2 video buffers of {} bytes",
               sizeof(VideoBase_t::VMEM));
  init_keymap();
  kbd_buf = std::make_unique<KBD_BUF>();
  std::promise<void> device_inited_promise;
  std::future<void> device_inited_future = device_inited_promise.get_future();
  device_thread = std::make_unique<std::jthread>(
      device_thread_work, std::ref(device_inited_promise));
  device_inited_future.get();
  spdlog::info("VGA thread initialization finished");
}

inline void init_audio() {
  Audio.sbuf = std::make_unique<AudioBase_t::SBF>();
  spdlog::info("Allocated sound buffer of {} bytes", sizeof(AudioBase_t::SBF));
}
inline std::unique_ptr<std::jthread> keyboard_thread;