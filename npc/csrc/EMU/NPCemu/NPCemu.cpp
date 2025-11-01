#include "Device/Device.h"
#include "VCPU.h"
#include "VCPU___024root.h"
#include "my_utils.h"
#include "verilated.h"
#include <Simulators/NPCemu.h>
#include <Simulators/RISCV32.h>
#include <cstdint>
#include <format>
#include <lockfree/lockfree.hpp>
#include <optional>
#include <print>
#include <stdexcept>
#include <string_view>
NPCemu::NPCemu(size_t MemSize, std::string_view program)
    : RISCV32(MemSize, program, Devices::PC_Init), trapped(0), context(),
      dut(&context), inst_count(0) {}

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

void NPCemu::step() {
  addr_t pc = dut.io_pc;
#ifndef DISABLE_ADDR_CHECK
  if (pc < Devices::memOffset) [[unlikely]] {
    log_and_throw<std::logic_error>("pc : {:08x} out of range", pc);
  }
#endif
  dut.io_instr = M[(pc - Devices::memOffset) / 4];

  uint32_t rs1 = (dut.io_instr >> 15) & 0x1f;

#ifndef DISABLE_ALL_TRACER
  if (tracer) [[unlikely]] {
    tracer->flush_instruction(dut.io_pc, dut.io_instr, getGPR(rs1));
  }
#endif

  dut.clock = 0;
  dut.eval();
  if (dut.io_valid) {
    dut.io_rdata = readMemory(dut.io_raddr);
  }
  dut.clock = 1;
  dut.eval();
  if (dut.io_wen && dut.io_valid) {
    writeMemory(dut.io_waddr, dut.io_wdata, dut.io_wmask);
    // extern bool mtracer;
    // if (mtracer) {
    //   println("Write to memory : {:#010x}, data : {:#010x}, mask : {:#010x}",
    //           (uint32_t)dut.io_waddr, (uint32_t)dut.io_wdata,
    //           (uint32_t)dut.io_wmask);
    // }
  }
  inst_count++;
  if (dut.io_ebreak) [[unlikely]] {
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
  log_and_throw<std::logic_error>("Invalid register index : {}", idx);
}

unsigned long long NPCemu::instrCount() { return inst_count; }

void NPCemu::writeMemory(int waddr, int wdata, char wmask) {
  for (int i = 0; i < 4; i++) {
    if ((wmask >> i) & 0x1) {
      uint32_t mask32 =
          (uint32_t)((1ull << (8ull * (i + 1))) - (1ull << (8ull * i)));
      uint32_t addr = (uint32_t)(waddr - Devices::memOffset) >> 2;
      if (addr < M.size() && waddr >= Devices::memOffset) {
        M[addr] &= ~mask32;
        M[addr] |= wdata & mask32;
      } else if (waddr >= Devices::deviceBase && devices) {
        devices->writeMMIO(waddr & ~0x3, mask32, wdata);
      }
    }
  }
}
uint32_t NPCemu::readMemory(int raddr) {
  raddr &= ~0x3;
  uint32_t addr = (uint32_t)(raddr - Devices::PC_Init) >> 2;
  if (addr < M.size() && raddr >= Devices::PC_Init)
    return M[addr];
  if (raddr >= Devices::deviceBase && devices) {
    auto ret = devices->readMMIO(raddr);
    return ret.value_or(0xdeadbeef);
  }

  return 0xdeafbeef;
}

NPCemu::~NPCemu() {}