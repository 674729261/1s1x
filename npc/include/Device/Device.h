#pragma once
#include <SDL2/SDL.h>
#include <Simulators/NPCDeviceBases/Audio.h>
#include <Simulators/NPCDeviceBases/Keyboard.h>
#include <Simulators/NPCDeviceBases/VGA.h>
#include <cstdint>
#include <lockfree/spsc/queue.hpp>
#include <memory>
#include <thread>
class Devices {
public:
  struct DeviceSettings {
    bool enable_vga;
    bool enable_audio;
    bool enable_keyboard;
  };

  Devices(DeviceSettings ds)
      : KeyboardBase({}), AudioBase({}), VideoBase(), RTC({}),
        device_settings(ds) {}

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

  const DeviceSettings device_settings;

  ~Devices();

private:
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
