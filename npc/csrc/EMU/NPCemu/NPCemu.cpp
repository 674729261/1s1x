#include "Device/Device.h"
#include "my_utils.h"
#include "verilated.h"
#include <Simulators/NPCemu.h>
#include <Simulators/RISCV32.h>
#include <Vnpc_top.h>
#include <Vnpc_top___024root.h>
#include <cstdint>
#include <format>
#include <lockfree/lockfree.hpp>
#include <print>
#include <verilated_vcd_c.h>

#define gprname(X) npc_top__DOT__cpu__DOT__gpr__DOT__register_bank_regs_##X##_r

NPCemu::NPCemu()
#include <stdexcept>
    : RISCV32(), trapped(0), context(), dut(&context), inst_count(0) {
  Verilated::traceEverOn(true);
  m_trace = new VerilatedVcdC;
  dut.trace(m_trace, 3);
  m_trace->open("waveform.vcd")
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

void NPCemu::handle_bus() {
  // if (dut.io_ifu_valid)
  //   dut.io_ifu_inst = devices->get_instruction(dut.io_ifu_addr);
  if (dut.io_lsu_valid) {
    if (dut.io_lsu_wen) {
      devices->writeMemory(dut.io_lsu_waddr, dut.io_lsu_wdata,
                           dut.io_lsu_wmask);
    } else {
      dut.io_lsu_rdata = devices->readMemory(dut.io_lsu_raddr);
      // print("?");
    }
  }
}

void NPCemu::step() {
  bool ready_to_step = false, is_ebreak = false;
  while (!ready_to_step) {
    // print("!");
    handle_bus();
    dut.clock = 0;
    dut.eval();
    ready_to_step = dut.io_ok_to_step;
    dut.clock = 1;
    dut.eval();

    if (dut.io_ebreak)
      is_ebreak = true;
    // std::print("!");
    // println("ok ! {}", ready_to_step);
  }
  inst_count++;
  if (is_ebreak) [[unlikely]] {
    EMUstate = RISCV32::Interrupt::EBREAK;
  }
  // std::println();
}

void NPCemu::syncCPUState() {
  cpu.gpr[0] = 0;
  cpu.gpr[1] = dut.rootp->gprname(0);
  cpu.gpr[2] = dut.rootp->gprname(1);
  cpu.gpr[3] = dut.rootp->gprname(2);
  cpu.gpr[4] = dut.rootp->gprname(3);
  cpu.gpr[5] = dut.rootp->gprname(4);
  cpu.gpr[6] = dut.rootp->gprname(5);
  cpu.gpr[7] = dut.rootp->gprname(6);
  cpu.gpr[8] = dut.rootp->gprname(7);
  cpu.gpr[9] = dut.rootp->gprname(8);
  cpu.gpr[10] = dut.rootp->gprname(9);
  cpu.gpr[11] = dut.rootp->gprname(10);
  cpu.gpr[12] = dut.rootp->gprname(11);
  cpu.gpr[13] = dut.rootp->gprname(12);
  cpu.gpr[14] = dut.rootp->gprname(13);
  cpu.gpr[15] = dut.rootp->gprname(14);
  cpu.gpr[16] = dut.rootp->gprname(15);
  cpu.gpr[17] = dut.rootp->gprname(16);
  cpu.gpr[18] = dut.rootp->gprname(17);
  cpu.gpr[19] = dut.rootp->gprname(18);
  cpu.gpr[20] = dut.rootp->gprname(19);
  cpu.gpr[21] = dut.rootp->gprname(20);
  cpu.gpr[22] = dut.rootp->gprname(21);
  cpu.gpr[23] = dut.rootp->gprname(22);
  cpu.gpr[24] = dut.rootp->gprname(23);
  cpu.gpr[25] = dut.rootp->gprname(24);
  cpu.gpr[26] = dut.rootp->gprname(25);
  cpu.gpr[27] = dut.rootp->gprname(26);
  cpu.gpr[28] = dut.rootp->gprname(27);
  cpu.gpr[29] = dut.rootp->gprname(28);
  cpu.gpr[30] = dut.rootp->gprname(29);
  cpu.gpr[31] = dut.rootp->gprname(30);
  cpu.pc = dut.io_pc;
}

uint32_t NPCemu::getGPR(int idx) {
  switch (idx) {
  case 0:
    return 0;
  case 1:
    return dut.rootp->gprname(0);
  case 2:
    return dut.rootp->gprname(1);
  case 3:
    return dut.rootp->gprname(2);
  case 4:
    return dut.rootp->gprname(3);
  case 5:
    return dut.rootp->gprname(4);
  case 6:
    return dut.rootp->gprname(5);
  case 7:
    return dut.rootp->gprname(6);
  case 8:
    return dut.rootp->gprname(7);
  case 9:
    return dut.rootp->gprname(8);
  case 10:
    return dut.rootp->gprname(9);
  case 11:
    return dut.rootp->gprname(10);
  case 12:
    return dut.rootp->gprname(11);
  case 13:
    return dut.rootp->gprname(12);
  case 14:
    return dut.rootp->gprname(13);
  case 15:
    return dut.rootp->gprname(14);
  case 16:
    return dut.rootp->gprname(15);
  case 17:
    return dut.rootp->gprname(16);
  case 18:
    return dut.rootp->gprname(17);
  case 19:
    return dut.rootp->gprname(18);
  case 20:
    return dut.rootp->gprname(19);
  case 21:
    return dut.rootp->gprname(20);
  case 22:
    return dut.rootp->gprname(21);
  case 23:
    return dut.rootp->gprname(22);
  case 24:
    return dut.rootp->gprname(23);
  case 25:
    return dut.rootp->gprname(24);
  case 26:
    return dut.rootp->gprname(25);
  case 27:
    return dut.rootp->gprname(26);
  case 28:
    return dut.rootp->gprname(27);
  case 29:
    return dut.rootp->gprname(28);
  case 30:
    return dut.rootp->gprname(29);
  case 31:
    return dut.rootp->gprname(30);
  case 32:
    return dut.io_pc;
  }
  log_and_throw<std::logic_error>("Invalid register index : {}", idx);
}

unsigned long long NPCemu::instrCount() { return inst_count; }

NPCemu::~NPCemu() { m_trace->close(); }