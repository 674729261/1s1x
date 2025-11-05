#include <Device/Device.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <my_utils.h>
#include <stdexcept>

Devices::Devices(Devices::DeviceSettings ds, size_t MemSize,
                 std::string_view program, bool mtracer)
    : KeyboardBase{}, AudioBase{}, VideoBase{}, RTC{}, renderer(nullptr),
      texture(nullptr), window(nullptr), quit(false), device_settings(ds),
      mtracer(mtracer), operation({.used = false}), multiple_emu(false) {
  using std::ifstream;
  using std::ios;

  M.resize(MemSize / 4);
  std::filesystem::path program_path = program;
  std::ifstream prog_file(program_path, ios::in | ios::binary);
  if (!prog_file.good()) {
    log_and_throw<std::runtime_error>("Failed to open program file {}",
                                      program);
  }
  uint32_t size_prog = 0;
  prog_file.seekg(0, ios::end);
  size_prog = prog_file.tellg();
  prog_file.seekg(0, ios::beg);
  if (size_prog > MemSize * sizeof(uint32_t))
    log_and_throw<std::logic_error>(
        "Program size is bigger than memory size {}", MemSize);

  prog_file.read(reinterpret_cast<char *>(M.data()), size_prog);

  spdlog::info("Loaded {} words", size_prog);
  // cpu.pc = init_pc;
  prog_file.close();
}

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

  using namespace std::chrono;
  auto last = steady_clock::now();
  try {
    while (device_alive) {
      if (device_running) {
        auto now = steady_clock::now();
        if (now - last < 16.67ms)
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