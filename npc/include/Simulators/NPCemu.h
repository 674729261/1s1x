#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>

#include "RISCV32.h"
#include <Simulators/NPCDeviceBases/Audio.h>
#include <Simulators/NPCDeviceBases/Keyboard.h>
#include <Simulators/NPCDeviceBases/VGA.h>
#include <VCPU.h>
#include <lockfree/spsc/queue.hpp>

extern "C" void trap(int signal);
extern "C" int pmem_read(int raddr, int clk, int valid);
extern "C" void pmem_write(int waddr, int wdata, char wmask);

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

class ProgSymTab;

class NPCemu : public RISCV32 {
public:
  struct DeviceSettings {
    bool enable_vga;
    bool enable_audio;
    bool enable_keyboard;
  };

  NPCemu(size_t MemSize, std::string_view programe,
         DeviceSettings dev_settings);

  addr_t getPC() override final;

  void reset() override final;
  void step() override final;
  unsigned long long instrCount() override final;

  void writeMemory(int waddr, int wdata, char wmask) override final;
  uint32_t readMemory(int raddr) override final;
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;
  void pause(bool is_paused) override final;

  friend void trap(int signal);
  friend int pmem_read(int raddr);
  friend void pmem_write(int waddr, int wdata, char wmask);

  static constexpr size_t SoundBufferSize = 0x10000;

  ~NPCemu();

private:
  static constexpr addr_t PC_Init = 0x80000000u;
  static constexpr addr_t memOffset = 0x80000000u;
  static constexpr addr_t deviceBase = 0xa0000000u;
  static constexpr addr_t RTCAddr = deviceBase + 0x0000048u;
  static constexpr addr_t RTCAddrEnd = RTCAddr + 0x8u;
  static constexpr addr_t SerialPort = deviceBase + 0x00003f8;
  static constexpr addr_t AudioPort = deviceBase + 0x0000200;
  static constexpr addr_t SoundBufferPort = deviceBase + 0x1200000;
  static constexpr addr_t VGAControlRegsPort = deviceBase + 0x0000100;
  static constexpr addr_t VGAFBPort = deviceBase + 0x1000000;
  static constexpr addr_t KeyboardPort = deviceBase + 0x0000060;

  static constexpr uint32_t ScreenWidth = 400;
  static constexpr uint32_t ScreenHeight = 300;
  static constexpr uint32_t VMemSize =
      ScreenWidth * ScreenHeight * sizeof(uint32_t);

  TOP_NAME dut;

  int trapped;
  unsigned long long inst_count;

  const DeviceSettings device_settings;

private:
  void writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata);
  std::optional<uint32_t> readMMIO(int raddr);

  struct {
    uint32_t RTC_reg[2];
    std::chrono::steady_clock::time_point last_time;
  } RTC;

  KeyboardBase_t KeyboardBase;
  AudioBase_t AudioBase;
  VideoBase_t VideoBase;

  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Window *window;

  void record_ftracer(uint32_t cur_inst, std::shared_ptr<ProgSymTab> sy_tab);

  void update_RTC();
  void init_ioe();

  std::atomic<bool> device_running, device_alive;
  std::thread device_update_thread;

  std::unique_ptr<lockfree::spsc::Queue<uint32_t, 1024>> key_queue;

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
};
