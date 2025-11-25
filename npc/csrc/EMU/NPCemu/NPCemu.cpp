#include "VCPU.h"
#include "VCPU___024root.h"
#include "my_utils.h"
#include "verilated.h"
#include <Simulators/NPCemu.h>
#include <Simulators/RISCV32.h>
#include <cstdint>
#include <exception>
#include <format>
#include <lockfree/lockfree.hpp>
#include <print>
NPCemu::NPCemu()
#include <stdexcept>
    : RISCV32(), trapped(0), context(), dut(&context), inst_count(0) {
}

RISCV32::addr_t NPCemu::getPC() { return getGPR(32); }

void NPCemu::reset() {
  dut.reset = 1;
  dut.clock = 0;
  dut.eval();
  dut.clock = 1;
  dut.eval();
  dut.clock = 0;
  dut.reset = 0;
  // dut.eval();

  syncCPUState();
}

void NPCemu::step() {
  bool ready_to_step = false;
  while (!ready_to_step) {
    ready_to_step = dut.io_ok_to_step;
    dut.clock = 0;
    dut.eval();
    if (!dut.io_wen && dut.io_valid) {
      dut.io_rdata = devices->readMemory(dut.io_raddr);
    }

    dut.clock = 1;

    dut.eval();
    if (dut.io_wen && dut.io_valid) {
      devices->writeMemory(dut.io_waddr, dut.io_wdata, dut.io_wmask);
    }
    addr_t pc = dut.io_pc;
    if (dut.io_ifu_valid) {
      dut.io_instr = devices->get_instruction(dut.io_ifu_addr);

      uint32_t rs1 = (dut.io_instr >> 15) & 0x1f;

#ifndef DISABLE_ALL_TRACER
      if (tracer) [[unlikely]] {
        tracer->flush_instruction(dut.io_ifu_addr, dut.io_instr, getGPR(rs1));
      }
#endif
    }
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

NPCemu::~NPCemu() {}