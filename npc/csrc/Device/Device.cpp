#include <Device/Device.h>
#include <iostream>
#include <memory>
#include <signal.h>
void Devices::init_ioe() {
  if (device_settings.enable_audio) {
    AudioBase.sbuf = std::make_unique_for_overwrite<uint8_t[]>(SoundBufferSize);
    AudioBase.reg_ctl.reg_sbuf_size = SoundBufferSize;
  }
  if (device_settings.enable_vga) {
    VideoBase.vmem1 = std::make_unique_for_overwrite<uint8_t[]>(VMemSize);
    VideoBase.vmem2 = std::make_unique_for_overwrite<uint8_t[]>(VMemSize);
    VideoBase.front_ptr = VideoBase.vmem1.get();
    VideoBase.back_ptr = VideoBase.vmem2.get();
    VideoBase.screen_size_info = (ScreenWidth << 16) | ScreenHeight;
    // init_vga();
  }
  if (device_settings.enable_keyboard) {
    key_queue = std::make_unique<lockfree::spsc::Queue<uint32_t, 1024>>();
  }
  device_alive = true;
  device_running = true;
  device_update_thread = std::thread(&Devices::device_update_loop, this);
}
void Devices::pause(bool is_paused) { device_running = !is_paused; }
void Devices::device_update_loop() {
  if (device_settings.enable_vga) {
    init_vga();
  }
  if (device_settings.enable_keyboard) {
    init_keyboard();
  }
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  using namespace std::chrono;
  auto last = steady_clock::now();
  try {
    while (device_alive) {
      if (device_running) {
        auto now = steady_clock::now();
        if (duration_cast<microseconds>(now - last).count() < 1'000'000 / 60)
          continue;
        last = now;
        if (device_settings.enable_vga) {
          vga_update_screen();
        }
        if (device_settings.enable_keyboard) {
          process_keyboard();
        }
      }
    }
  } catch (const std::exception &err) {
    std::println(std::cerr, "Error : {}", err.what());
    std::terminate();
  }
  if (texture)
    SDL_DestroyTexture(texture);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
}

void Devices::update_RTC() {
  using namespace std::chrono;
  auto now_tick = steady_clock().now();
  uint64_t duration = static_cast<uint64_t>(
      duration_cast<microseconds>(now_tick - RTC.last_time).count());
  // uint64_t start_time =
  //     RTC.RTC_reg[0] | (static_cast<uint64_t>(RTC.RTC_reg[1]) << 32);
  uint64_t now_time = duration;

  RTC.RTC_reg[0] = static_cast<uint32_t>(now_time & 0xFFFFFFFF);
  RTC.RTC_reg[1] = static_cast<uint32_t>(now_time >> 32);
  // RTC.last_time = now_tick;
  // std::print("!!{}\r", now_time);
}

Devices::~Devices() {
  device_running = false;
  device_alive = false;
  device_update_thread.join();

  SDL_CloseAudio();
  SDL_Quit();
}