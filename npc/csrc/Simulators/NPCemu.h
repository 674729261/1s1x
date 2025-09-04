#pragma once
#include "RISCV32.h"
#include <VCPU.h>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" void trap(int signal);
extern "C" int pmem_read(int raddr);
extern "C" void pmem_write(int waddr, int wdata, char wmask);

class NPCemu : public RISCV32 {
public:
  NPCemu(size_t MemSize, std::string_view programe);

  addr_t getPC() override final;

  void reset() override final;
  void step(bool display = false) override final;
  int instrCount() override final;

  void writeMemory(int waddr, int wdata, char wmask) override final;
  uint32_t readMemory(int raddr) override final;
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;

  friend void trap(int signal);
  friend int pmem_read(int raddr);
  friend void pmem_write(int waddr, int wdata, char wmask);

private:
  const addr_t PC_Init = 0x80000000u;
  const addr_t memOffset = 0x80000000u;
  const addr_t deviceBase = 0xa0000000u;
  const addr_t RTCAddr = deviceBase + 0x0000048u;
  const addr_t RTCAddrEnd = RTCAddr + 0x8u;
  const addr_t SerialPort = (deviceBase + 0x00003f8);

  TOP_NAME dut;

  int trapped;
  int inst_count;

private:
  void writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata);
  std::optional<uint32_t> readMMIO(int raddr);

  struct {
    uint32_t RTC_reg[2];
    std::chrono::steady_clock::time_point last_time;
  } RTC;

  void update_RTC();
};
