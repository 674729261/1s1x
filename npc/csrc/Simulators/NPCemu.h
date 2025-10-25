#pragma once
#include "RISCV32.h"
#include <SDL2/SDL.h>
#include <VCPU.h>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string_view>

extern "C" void trap(int signal);
extern "C" int pmem_read(int raddr);
extern "C" void pmem_write(int waddr, int wdata, char wmask);

class NPCemu : public RISCV32 {
public:
  struct DeviceSettings {
    bool enable_vga;
    bool enable_audio;
    bool enable_keyboard;
  };
  struct AudioBase_t {
    uint32_t reg_freq;
    uint32_t reg_channels;
    uint32_t reg_samples;
    uint32_t reg_sbuf_size;
    uint32_t reg_init;
    uint32_t reg_count;
    static constexpr int n_regs = 6;

    std::unique_ptr<uint8_t[]> sbuf;
  };

  NPCemu(size_t MemSize, std::string_view programe,
         DeviceSettings dev_settings);

  addr_t getPC() override final;

  void reset() override final;
  void step(bool display = false, bool record_inst = false,
            bool ftracer = false) override final;
  int instrCount() override final;

  void writeMemory(int waddr, int wdata, char wmask) override final;
  uint32_t readMemory(int raddr) override final;
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;

  friend void trap(int signal);
  friend int pmem_read(int raddr);
  friend void pmem_write(int waddr, int wdata, char wmask);
  static constexpr size_t SoundBufferSize = 0x10000;

private:
  static constexpr addr_t PC_Init = 0x80000000u;
  static constexpr addr_t memOffset = 0x80000000u;
  static constexpr addr_t deviceBase = 0xa0000000u;
  static constexpr addr_t RTCAddr = deviceBase + 0x0000048u;
  static constexpr addr_t RTCAddrEnd = RTCAddr + 0x8u;
  static constexpr addr_t SerialPort = (deviceBase + 0x00003f8);
  static constexpr addr_t AudioPort = (deviceBase + 0x0000200);
  static constexpr addr_t SoundBufferPort = (deviceBase + 0x1200000);

  TOP_NAME dut;

  int trapped;
  int inst_count;

  DeviceSettings device_settings;

private:
  void writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata);
  std::optional<uint32_t> readMMIO(int raddr);

  struct {
    uint32_t RTC_reg[2];
    std::chrono::steady_clock::time_point last_time;
  } RTC;

  AudioBase_t AudioBase;

  void record_ftracer(uint32_t cur_inst);

  void update_RTC();
  void init_ioe();

  void init_audio();
  void init_keyboard();
  void init_vga();
};
