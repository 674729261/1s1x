#pragma once

#include "Device/Audio.h"
#include "Device/RTC.h"
#include "spdlog/spdlog.h"
#include <Device/VGA.h>
#include <SDL2/SDL.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
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

inline std::unique_ptr<std::jthread> vga_thread;
inline std::atomic_bool quit;
inline void vga_thread_work(std::promise<void> &vga_inited_promise) {
  static SDL_Renderer *renderer = NULL;
  static SDL_Texture *texture = NULL;
  SDL_Window *window = NULL;
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(VideoBase_t::ScreenWidth * 2,
                              VideoBase_t::ScreenHeight * 2, 0, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "RISCV32E-NPC");
  texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
      VideoBase_t::ScreenWidth, VideoBase_t::ScreenHeight);
  SDL_RenderPresent(renderer);
  vga_inited_promise.set_value();
  using namespace std::chrono_literals;
  auto last_tick = std::chrono::steady_clock::now();
  while (!quit.load()) {
    auto cur_tick = std::chrono::steady_clock::now();
    if (Video.sync.load() && cur_tick - last_tick >= 16.667ms) {
      last_tick = cur_tick;
      uint8_t *ptr_vmem = Video.front_ptr.load();
      SDL_UpdateTexture(texture, NULL, ptr_vmem,
                        VideoBase_t::ScreenWidth * sizeof(uint32_t));
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
      Video.sync.store(0);
    }
  }
}

inline void init_vga() {
  Video.vmem1 = std::make_unique<VideoBase_t::VMEM>();
  Video.vmem2 = std::make_unique<VideoBase_t::VMEM>();

  spdlog::info("Allocated 2 video buffers of {} bytes",
               sizeof(VideoBase_t::VMEM));

  std::promise<void> vga_inited_promise;
  std::future<void> vga_inited_future = vga_inited_promise.get_future();
  vga_thread = std::make_unique<std::jthread>(vga_thread_work,
                                              std::ref(vga_inited_promise));
  vga_inited_future.get();
  spdlog::info("VGA thread initialization finished");
}

inline void init_audio() {
  Audio.sbuf = std::make_unique<AudioBase_t::SBF>();
  spdlog::info("Allocated sound buffer of {} bytes", sizeof(AudioBase_t::SBF));
}
inline std::unique_ptr<std::jthread> keyboard_thread;