
#include "DUT.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <MROM.h>
#include <VysyxSoCFull.h>
#include <VysyxSoCFull___024root.h>
#include <exception>
#include <iostream>
#include <memory>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>

using std::string;

int simulate(int argc, char *argv[], Config config) {
  Verilated::commandArgs(argc, argv);
  std::unique_ptr<VerilatedContext> contextp =
      std::make_unique<VerilatedContext>();
  contextp->commandArgs(argc, argv);

  Verilated::traceEverOn(true);

  Dut dut(config, contextp.get());
  dut.reset();
  while (!contextp->gotFinish() && dut.sim_time <= 2 * 10000) {
    dut.step_one_cycle();
    println("PC = {:#010x}", dut.getPC());
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
  } else {
    spdlog::warn("FAILED TO HALT");
    result = -2;
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