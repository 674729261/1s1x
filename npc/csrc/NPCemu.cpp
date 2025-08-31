#include "Simulators/NPCemu.h"
#include "Simulators/RISCV32.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string_view>

NPCemu::NPCemu(size_t MemSize, std::string_view program)
    : RISCV32(MemSize, program, PC_Init), dut(), trapped(0), inst_count(0) {}

RISCV32::CPU_State NPCemu::getCPUState() { return cpu; }

uint32_t NPCemu::getGPR(int idx) {
  assert(idx >= 0 && idx < 32);
  return cpu.gpr[idx];
}
RISCV32::addr_t NPCemu::getPC() { return cpu.pc; }

void NPCemu::reset() {
  dut.reset = 1;
  dut.clock = 0;
  dut.eval();
  dut.clock = 1;
  dut.eval();
  dut.clock = 0;
  dut.reset = 0;
  dut.eval();
}

RISCV32::Interrupt NPCemu::step(std::size_t c) {
  addr_t pc = dut.io_pc;
  if (pc < memOffset) {
    throw std::logic_error(std::format("pc : {:08x} out of range", pc));
  }
  dut.io_instr = M[(pc - memOffset) / 4];
  dut.clock = 0;
  dut.eval();
  dut.clock = 1;
  dut.eval();
  inst_count++;
  if (trapped)
    return RISCV32::Interrupt::EBREAK;

  return RISCV32::Interrupt::NONE;
}
int NPCemu::instrCount() { return inst_count; }

void NPCemu::writeMemory(int waddr, int wdata, char wmask) {
  for (int i = 0; i < 4; i++) {
    if ((wmask >> i) & 0x1) {
      uint32_t mask32 =
          (uint32_t)((1ull << (8ull * (i + 1))) - (1ull << (8ull * i)));
      uint32_t addr = (uint32_t)(waddr - memOffset) >> 2;
      if (addr < M.size() && waddr >= memOffset) {
        M[addr] &= ~mask32;
        M[addr] |= wdata & mask32;
      } else if (waddr >= deviceBase) {
        writeMMIO(waddr & ~0x3, mask32, wdata);
      }
    }
  }
}
uint32_t NPCemu::readMemory(int raddr) {
  raddr &= ~0x3;
  uint32_t addr = (uint32_t)(raddr - PC_Init) >> 2;
  if (addr < M.size() && raddr >= PC_Init)
    return M[addr];
  if (raddr >= deviceBase) {
    auto ret = readMMIO(raddr);
    if (!ret.has_value()) {
      return 0xdeafbeef;
    }
    return ret.value();
  }

  return 0xdeafbeef;
}

static void write_mask(uint32_t &dst, uint32_t mask32, uint32_t wdata) {
  dst &= ~mask32;
  dst |= wdata & mask32;
}

std::optional<uint32_t> NPCemu::readMMIO(int raddr) {
  if (raddr >= RTCAddr && raddr < RTCAddrEnd) {
    update_RTC();
    if (raddr == RTCAddr)
      return RTC.RTC_reg[0];
    else
      return RTC.RTC_reg[1];
  }
  return std::nullopt;
}

void NPCemu::update_RTC() {
  using namespace std::chrono;
  auto now_tick = steady_clock().now();
  uint64_t duration = static_cast<uint64_t>(
      duration_cast<microseconds>(now_tick - RTC.last_time).count());
  uint64_t start_time =
      RTC.RTC_reg[0] | (static_cast<uint64_t>(RTC.RTC_reg[1]) << 32);
  uint64_t now_time = start_time + duration;
  RTC.RTC_reg[0] = now_time & 0xFFFFFF;
  RTC.RTC_reg[1] = now_time >> 32;
  RTC.last_time = now_tick;
}

void NPCemu::writeMMIO(uint32_t waddr, uint32_t mask32, uint32_t wdata) {
  using namespace std::chrono;
  if (waddr >= RTCAddr && waddr < RTCAddrEnd) {
    uint32_t RTC_id = (waddr - RTCAddr) >> 2;
    update_RTC();
    write_mask(RTC.RTC_reg[RTC_id], mask32, wdata);
    RTC.last_time = steady_clock().now();
    return;
  }
  if (waddr == SerialPort) {
    if (mask32 != 0xFF)
      throw std::logic_error(std::format(
          "mask32 {:08x} is not 0xFF when writing serial port", mask32));
    std::cout.put(wdata);
    return;
  }
  throw std::logic_error(std::format("Writing to invalid MMIO {:08x}", waddr));
}