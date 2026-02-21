
#include "DUT.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <Mem.h>
#include <Ref.h>
#include <Vnpc_top.h>
#include <Vnpc_top___024root.h>
#include <chrono>
#include <memory>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>
std::unique_ptr<Dut> dut;
static bool retire;

extern "C" void notify_retire(int32_t pc, int32_t inst) { retire = true; }

bool check_difftest(Dut &dut, Ref &ref) {
  bool ret = false;
  for (int i = 0; i < 16; i++) {
    if (dut.getGPR(i) != ref.cpu.gpr[i]) {
      spdlog::error("gpr {} differs from ref : should be {:08x}, got {:08x}",
                    gpr_names[i], ref.cpu.gpr[i], dut.getGPR(i));
      ret = true;
    }
  }
  if (dut.getPC() != ref.cpu.pc) {
    spdlog::error("PC differs from ref : should be {:08x}, got {:08x}",
                  ref.cpu.pc, dut.getPC());
    ret = true;
  }

  return ret;
}

int simulate(int argc, char *argv[]) {
  // init_mrom(config.image_path);
  init_mem(config.image_path);
  Verilated::commandArgs(argc, argv);
  std::unique_ptr<VerilatedContext> contextp =
      std::make_unique<VerilatedContext>();
  contextp->commandArgs(argc, argv);

  Verilated::traceEverOn(config.use_waveform);

  dut = std::make_unique<Dut>(contextp.get());
  // Ref ref(*dut);
  // ref.reset(*dut);
  // dut->reset();
  bool difftest_state = false;
  auto start_time = std::chrono::steady_clock::now();
  // while (!contextp->gotFinish()) {
  //   retire = false;
  //   if (dut->top->rootp->npc_top__DOT__reset) {
  //     ref.reset(*dut);
  //     ref.sync_state();
  //   }
  //   // std::println("PC = {:08x}", dut->getPC());
  //   dut->step_one_cycle();

  //   if (retire) {
  //     // std::println("{:08x}", ref.getPC());
  //     if (config.difftest) {
  //       ref.step();
  //       difftest_state = check_difftest(*dut, ref);
  //       if (difftest_state)
  //         break;
  //     }
  //   }
  // }
  auto end_time = std::chrono::steady_clock::now();

  // int result;
  // if (contextp->gotFinish()) {
  //   if (dut->getGPR(10) == 0) {
  //     spdlog::info("HIT GOOD TRAP");
  //     result = 0;
  //   } else {
  //     spdlog::warn("HIT BAD TRAP with a0 = {:010x}", dut->getGPR(10));
  //     result = -1;
  //   }
  // } else if (difftest_state) {
  //   spdlog::warn("DIFFTEST FAILED");
  //   result = -2;
  // } else {
  //   spdlog::warn("FAILED TO HALT");
  //   result = -3;
  // }
  // dut->print_all_gpr();

  // dut = nullptr;
  return 0;
}