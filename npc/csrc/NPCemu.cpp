#include "Simulators/NPCemu.h"
#include "Capstone.h"
#include "RingBuffer.hpp"
#include "Simulators/RISCV32.h"
#include "Symbols.h"
#include "VCPU___024root.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <optional>
#include <ostream>
#include <print>
#include <stdexcept>
#include <string_view>
NPCemu::NPCemu(size_t MemSize, std::string_view program)
    : RISCV32(MemSize, program, PC_Init), dut("DUT"), trapped(0),
      inst_count(0) {}

RISCV32::addr_t NPCemu::getPC() { return getGPR(32); }

void NPCemu::reset() {
  dut.reset = 1;
  dut.clock = 0;
  dut.eval();
  dut.clock = 1;
  dut.eval();
  dut.clock = 0;
  dut.reset = 0;
  dut.eval();
  syncCPUState();
}

void NPCemu::step(bool display, bool record_inst, bool ftracer) {
  addr_t pc = dut.io_pc;
  if (pc < memOffset) {
    throw std::logic_error(std::format("pc : {:08x} out of range", pc));
  }
  dut.io_instr = M[(pc - memOffset) / 4];
  uint32_t cur_inst = dut.io_instr;
  if (display) {
    Capstone::capstone.disassemble(pc, (uint8_t *)&cur_inst, 4);
  }
  if (record_inst) {
    InstRingBuffer::instRingBuffer.insert(dut.io_pc, cur_inst);
  }
  if (ftracer) {
    uint32_t opcode = cur_inst & 0x7f;
    uint32_t rd = (cur_inst >> 7) & 0x1f;
    uint32_t dnxt_pc = -1;
    if (opcode == 0x6f) {
      uint32_t imm20 = cur_inst >> 31;
      uint32_t imm10_1 = (cur_inst >> 21) & 0x3ff;
      uint32_t imm11 = (cur_inst >> 20) & 0x1;
      uint32_t imm19_12 = (cur_inst >> 12) & 0xff;
      uint32_t imm =
          (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
      imm |= -(imm & 0x80000);
      dnxt_pc = (dut.io_pc + imm) & ~0x1;
    } else if (opcode == 0x67) {
      uint32_t rs1 = (cur_inst >> 15) & 0x1f;
      uint32_t imm = cur_inst >> 20;
      imm |= -(imm & 0x800);
      dnxt_pc = (imm + getGPR(rs1)) & ~0x1;
    }

    if ((opcode == 0x67 || opcode == 0x6f) && rd == 1) {
      for (int i = 0; i < cnt_stack_ftrace; i++)
        std::print(" ");
      int to_symbol = find_symbol(dnxt_pc);
      std::println("call {}@{:#010x}", find_symbol_name(to_symbol), dut.io_pc);
      push_stack_ftrace(dut.io_pc, to_symbol);
    } else if (cur_inst == 0x00008067) {
      Call top = pop_stack_ftrace();
      for (int i = 0; i < cnt_stack_ftrace; i++)
        std::print(" ");
      std::println("ret  {}@{:#010x}", find_symbol_name(top.symbol), dut.io_pc);
    }
  }

  dut.clock = 0;
  dut.eval();
  dut.clock = 1;
  dut.eval();
  inst_count++;
  if (trapped) {
    EMUstate = RISCV32::Interrupt::EBREAK;
  }
}

void NPCemu::syncCPUState() {
  cpu.gpr[0] = 0;
  cpu.gpr[1] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_0_r;
  cpu.gpr[2] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_1_r;
  cpu.gpr[3] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_2_r;
  cpu.gpr[4] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_3_r;
  cpu.gpr[5] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_4_r;
  cpu.gpr[6] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_5_r;
  cpu.gpr[7] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_6_r;
  cpu.gpr[8] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_7_r;
  cpu.gpr[9] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_8_r;
  cpu.gpr[10] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_9_r;
  cpu.gpr[11] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_10_r;
  cpu.gpr[12] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_11_r;
  cpu.gpr[13] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_12_r;
  cpu.gpr[14] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_13_r;
  cpu.gpr[15] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_14_r;
  cpu.gpr[16] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_15_r;
  cpu.gpr[17] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_16_r;
  cpu.gpr[18] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_17_r;
  cpu.gpr[19] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_18_r;
  cpu.gpr[20] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_19_r;
  cpu.gpr[21] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_20_r;
  cpu.gpr[22] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_21_r;
  cpu.gpr[23] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_22_r;
  cpu.gpr[24] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_23_r;
  cpu.gpr[25] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_24_r;
  cpu.gpr[26] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_25_r;
  cpu.gpr[27] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_26_r;
  cpu.gpr[28] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_27_r;
  cpu.gpr[29] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_28_r;
  cpu.gpr[30] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_29_r;
  cpu.gpr[31] = dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_30_r;
  cpu.pc = dut.io_pc;
}

uint32_t NPCemu::getGPR(int idx) {
  switch (idx) {
  case 0:
    return 0;
  case 1:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_0_r;
  case 2:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_1_r;
  case 3:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_2_r;
  case 4:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_3_r;
  case 5:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_4_r;
  case 6:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_5_r;
  case 7:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_6_r;
  case 8:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_7_r;
  case 9:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_8_r;
  case 10:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_9_r;
  case 11:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_10_r;
  case 12:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_11_r;
  case 13:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_12_r;
  case 14:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_13_r;
  case 15:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_14_r;
  case 16:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_15_r;
  case 17:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_16_r;
  case 18:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_17_r;
  case 19:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_18_r;
  case 20:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_19_r;
  case 21:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_20_r;
  case 22:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_21_r;
  case 23:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_22_r;
  case 24:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_23_r;
  case 25:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_24_r;
  case 26:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_25_r;
  case 27:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_26_r;
  case 28:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_27_r;
  case 29:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_28_r;
  case 30:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_29_r;
  case 31:
    return dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_30_r;
  case 32:
    return dut.io_pc;
  }
  throw std::logic_error(std::format("Invalid register index : {}", idx));
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

  RTC.RTC_reg[0] = now_time & 0xFFFFFFFF;
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
    std::cout.flush();
    return;
  }
  throw std::logic_error(std::format("Writing to invalid MMIO {:08x}", waddr));
}