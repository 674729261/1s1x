#pragma once
#include "my_utils.h"
#include <Device/Audio.h>
#include <Device/Keyboard.h>
#include <Device/VGA.h>
#include <SDL2/SDL.h>
#include <atomic>
#include <cstdint>
#include <lockfree/spsc/queue.hpp>
#include <memory>
#include <thread>
#include <vector>
class Devices {
public:
  struct DeviceSettings {
    bool enable_vga;
    bool enable_audio;
    bool enable_keyboard;
  };
  friend class NEMUemu;
  Devices(DeviceSettings ds, size_t MemSize, std::string_view program,
          bool mtracer);

  void writeMemory(uint32_t waddr, uint32_t wdata, uint32_t wmask);
  uint32_t readMemory(uint32_t raddr);
  uint32_t get_instruction(uint32_t pc) {
#ifndef DISABLE_ADDR_CHECK
    if (pc < Devices::memOffset) [[unlikely]] {
      log_and_throw<std::logic_error>("pc : {:08x} out of range", pc);
    }
#endif
    return M[(pc - Devices::memOffset) >> 2];
  }

  void writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata);
  std::optional<uint32_t> readMMIO(int raddr);
  void pause(bool is_paused);
  void update_RTC();
  void init_ioe();

  static void init_keymap();

  void device_update_loop();

  void init_audio();
  void init_keyboard();
  void init_vga();
  void vga_update_screen();
  void update_screen();

  void process_keyboard();

  void ensure_audio_enabled();
  void ensure_vga_enabled();

  bool is_quit() { return quit.load(std::memory_order_acquire); }
  void reset_quit() { quit.store(false); }
  void reset_op() { operation.used = false; }

  const DeviceSettings device_settings;

  ~Devices();

private:
  std::vector<uint32_t> M;
  bool mtracer;

  struct OP {
    bool used;
    struct OP_record {
      bool is_read;
      uint32_t addr;
      uint32_t wdata;
      uint32_t wmask;

      bool operator==(const OP_record &o) const {
        return is_read == o.is_read && addr == o.addr && wdata == o.wdata &&
               wmask == o.wmask;
      }
    } op;
    uint32_t rdata;
  } operation;

  struct {
    uint32_t RTC_reg[2];
    std::chrono::steady_clock::time_point last_time;
  } RTC;
  KeyboardBase_t KeyboardBase;
  AudioBase_t AudioBase;
  VideoBase_t VideoBase;

  std::atomic<bool> quit;

  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Window *window;

  std::atomic<bool> device_running, device_alive;
  std::thread device_update_thread;

  std::unique_ptr<lockfree::spsc::Queue<uint32_t, 1024>> key_queue;

public:
  static constexpr size_t SoundBufferSize = 0x10000;
  static constexpr uint32_t PC_Init = 0x80000000u;
  static constexpr uint32_t memOffset = 0x80000000u;
  static constexpr uint32_t deviceBase = 0xa0000000u;
  static constexpr uint32_t RTCAddr = deviceBase + 0x0000048u;
  static constexpr uint32_t RTCAddrEnd = RTCAddr + 0x8u;
  static constexpr uint32_t SerialPort = deviceBase + 0x00003f8;
  static constexpr uint32_t AudioPort = deviceBase + 0x0000200;
  static constexpr uint32_t SoundBufferPort = deviceBase + 0x1200000;
  static constexpr uint32_t VGAControlRegsPort = deviceBase + 0x0000100;
  static constexpr uint32_t VGAFBPort = deviceBase + 0x1000000;
  static constexpr uint32_t KeyboardPort = deviceBase + 0x0000060;

  static constexpr uint32_t ScreenWidth = 400;
  static constexpr uint32_t ScreenHeight = 300;
  static constexpr uint32_t VMemSize =
      ScreenWidth * ScreenHeight * sizeof(uint32_t);
};
