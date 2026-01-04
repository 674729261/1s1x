
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

void reset_soc(VysyxSoCFull &soc) {
  soc.reset = 1;
  soc.clock = 0;
  soc.eval();
  soc.clock = 1;
  soc.eval();
  soc.clock = 0;
  soc.eval();
  soc.clock = 1;
  soc.eval();
  soc.clock = 0;
  soc.eval();
  soc.clock = 1;
  soc.eval();
  soc.reset = 0;
}
#define gprname(X)                                                             \
  rootp                                                                        \
      ->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__cpu__DOT__gpr__DOT__register_bank_regs_##X##_r
int simulate(int argc, char *argv[], Config config) {
  Verilated::commandArgs(argc, argv);
  std::unique_ptr<VerilatedContext> contextp =
      std::make_unique<VerilatedContext>();
  contextp->commandArgs(argc, argv);
  std::unique_ptr<VysyxSoCFull> dut =
      std::make_unique<VysyxSoCFull>(contextp.get());
  std::unique_ptr<VerilatedVcdC> m_trace = std::make_unique<VerilatedVcdC>();
  Verilated::traceEverOn(true);
  dut->trace(m_trace.get(), 5);
  m_trace->open("waveform.vcd");
  vluint64_t sim_time = 0;

  init_mrom(config.image_path);
  reset_soc(*dut);
  while (!contextp->gotFinish() && sim_time <= 2 * 10000) {
    dut->clock = 0;
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
    dut->clock = 1;
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
  }
  int result;
  if (contextp->gotFinish()) {
    if (dut->gprname(10) == 0) {
      spdlog::info("HIT GOOD TRAP");
      result = 0;
    } else {
      spdlog::info("HIT BAD TRAP with a0 = {:010x}", dut->gprname(10));
      result = -1;
    }
  } else {
    spdlog::info("FAILED TO HALT");
    result = -2;
  }
  m_trace->close();

  return result;
}

int main(int argc, char *argv[]) {
  Config config = process_args(argc, argv);
  int return_value = -1;
  try {
    simulate(argc, argv, config);
  } catch (std::exception e) {
    std::println(std::cerr, "Error : {}", e.what());
  }
  spdlog::shutdown();
}