
#include "DUT.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <MROM.h>
#include <Ref.h>
#include <VysyxSoCFull.h>
#include <VysyxSoCFull___024root.h>
#include <exception>
#include <iostream>
#include <memory>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>

using std::string;

bool retire;

extern "C" void notify_retire(int32_t pc, int32_t inst) { retire = true; }

bool check_difftest(Dut &dut, Ref &ref) {
  bool ret = false;
  for (int i = 0; i < 32; i++) {
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

int simulate(int argc, char *argv[], Config config) {
  Verilated::commandArgs(argc, argv);
  std::unique_ptr<VerilatedContext> contextp =
      std::make_unique<VerilatedContext>();
  contextp->commandArgs(argc, argv);

  Verilated::traceEverOn(true);

  Dut dut(config, contextp.get());
  Ref ref(config);
  dut.reset();
  ref.reset();
  bool difftest_state = false;
  while (!contextp->gotFinish()) {
    retire = false;
    if (dut.top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__reset)
      ref.reset();
    dut.step_one_cycle();
    println(
        "{:6} {:08x} {} {:08x}", retire, dut.getPC(),
        dut.top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__reset,
        ref.getPC());
    if (retire) {
      ref.step();
      if (config.difftest) {
        difftest_state = check_difftest(dut, ref);
        if (difftest_state)
          break;
      }
    }
  }
  int result;
  if (contextp->gotFinish()) {
    if (dut.getGPR(10) == 0) {
      spdlog::info("HIT GOOD TRAP");
      result = 0;
    } else {
      spdlog::warn("HIT BAD TRAP with a0 = {:010x}", dut.getGPR(10));
      result = -1;
    }
  } else if (difftest_state) {
    spdlog::warn("DIFFTEST FAILED");
    result = -2;
  } else {
    spdlog::warn("FAILED TO HALT");
    result = -3;
  }
  dut.print_all_gpr();
  return result;
}

int main(int argc, char *argv[]) {
  Config config = process_args(argc, argv);
  int return_value = -1;
  try {
    return_value = simulate(argc, argv, config);
  } catch (std::exception e) {
    std::println(std::cerr, "Error : {}", e.what());
  }
  spdlog::shutdown();
  return return_value;
}