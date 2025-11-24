#include <Device/Device.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_error.h>
#include <cstdint>
#include <my_utils.h>
#include <print>
#include <spdlog/spdlog.h>
#include <stdexcept>

void Devices::ensure_vga_enabled() {
  if (!device_settings.enable_vga) {
    log_and_throw<std::logic_error>(
        "Accessing audio MMIO when vga is disabled");
  }
}

void Devices::vga_update_screen() {
  if (VideoBase.sync) {
    update_screen();
    VideoBase.sync = 0;
  }
}

void Devices::update_screen() {
  uint8_t *ptr_vmem = VideoBase.front_ptr.load();
  SDL_UpdateTexture(texture, NULL, ptr_vmem, ScreenWidth * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void Devices::init_vga() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_CreateWindowAndRenderer(ScreenWidth * 2, ScreenHeight * 2, 0, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "RISCV32-NPC");
  std::println("??{}!!", SDL_GetError());
  texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STATIC, ScreenWidth, ScreenHeight);
  SDL_RenderPresent(renderer);
}