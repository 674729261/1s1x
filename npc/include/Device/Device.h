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

  void set_multiple_emu() { multiple_emu = true; }

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
  bool multiple_emu;

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

constexpr std::array<uint32_t, 16> lookup_mask32 = {
    0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF, 0x00FF0000, 0x00FF00FF,
    0x00FFFF00, 0x00FFFFFF, 0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
    0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF};

inline void Devices::writeMemory(uint32_t waddr, uint32_t wdata,
                                 uint32_t wmask) {
  uint32_t mask32 = lookup_mask32[wmask];
#ifndef NO_DIFFTEST
  if (multiple_emu) [[unlikely]] {
    if (operation.used) {
      wdata &= mask32;
      if (operation.op !=
          OP::OP_record{
              .is_read = false, .addr = waddr, .wdata = wdata, .wmask = wmask})
        log_and_throw<std::logic_error>(
            "Different memory operation from ref\n"
            "dut : {:6} addr={:#010x} data={:#010x} mask={:x}\n"
            "ref : {:6} addr={:#10x} data={:#010x} mask={:x}",
            operation.op.is_read ? "read" : "write", operation.op.addr,
            operation.op.wdata, operation.op.wmask, "write", waddr, wdata,
            wmask);
      return;
    } else {
      operation.used = true;
      operation.op = {.is_read = false,
                      .addr = waddr,
                      .wdata = wdata & mask32,
                      .wmask = wmask};
    }
  }
#endif
#ifndef DISABLE_ALL_TRACER
  if (mtracer) {
    println("Write to memory : {:#010x}, data : {:#010x}, mask : {:#010x}",
            (uint32_t)waddr, (uint32_t)wdata, (uint32_t)wmask);
  }
#endif

  uint32_t addr = (uint32_t)(waddr - Devices::memOffset) >> 2;
  if (addr < M.size() && waddr >= Devices::memOffset) [[likely]] {
    M[addr] &= ~mask32;
    M[addr] |= wdata & mask32;
  } else if (waddr >= Devices::deviceBase) {
    writeMMIO(waddr & ~0x3, mask32, wdata);
  }
}

inline uint32_t Devices::readMemory(uint32_t raddr) {
  raddr &= ~0x3;
#ifndef NO_DIFFTEST
  if (multiple_emu) [[unlikely]] {
    if (operation.used) {
      if (operation.op.is_read != true || operation.op.addr != raddr)
        log_and_throw<std::logic_error>(
            "Different memory operation from ref\n"
            "dut : {:6} addr={:#010x} data={:#010x} mask={:x}\n"
            "ref : {:6} addr={:#10x}",
            operation.op.is_read ? "read" : "write", operation.op.addr,
            operation.op.wdata, operation.op.wmask, "read", raddr);
      return operation.rdata;
    }
    operation.used = true;
    operation.op.is_read = true;
    operation.op.addr = raddr;
  }
#endif

#ifndef DISABLE_ALL_TRACER
  if (mtracer) {
    println("Reading memory memory : {:#010x}", (uint32_t)raddr);
  }
#endif
  uint32_t rdata = 0xdeadbeef;

  uint32_t addr = (uint32_t)(raddr - Devices::PC_Init) >> 2;
  if (addr < M.size() && raddr >= Devices::PC_Init) [[likely]]
    rdata = M[addr];
  else if (raddr >= Devices::deviceBase)
    rdata = readMMIO(raddr).value_or(0xdeadbeef);
#ifndef NO_DIFFTEST
  operation.rdata = rdata;
#endif
  return rdata;
}
