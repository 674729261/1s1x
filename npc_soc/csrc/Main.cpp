#include "Setup.h"
#include <VysyxSoCFull.h>
#include <iostream>
#include <memory>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>

using std::cerr;
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

int main(int argc, char *argv[]) {
  Verilated::commandArgs(argc, argv);
  argparse::ArgumentParser program("NPCemu");
  register_argparse(program);
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    cerr << program;
    std::terminate();
  }
  Config config;
  try {
    register_logger(program);
    config = setup(program);
  } catch (const std::exception &err) {
    std::println(std::cerr, "Error : {}", err.what());
    std::terminate();
  }
  Verilated::traceEverOn(true);
  std::unique_ptr<VerilatedContext> contextp =
      std::make_unique<VerilatedContext>();
  contextp->commandArgs(argc, argv);
  std::unique_ptr<VysyxSoCFull> dut =
      std::make_unique<VysyxSoCFull>(contextp.get());
  std::unique_ptr<VerilatedVcdC> m_trace = std::make_unique<VerilatedVcdC>();
  dut->trace(m_trace.get(), 5);
  m_trace->open("waveform.vcd");
  static vluint64_t sim_time = 0;
  while (!contextp->gotFinish() && sim_time <= 2 * 1000000) {
    dut->clock = 0;
    dut->eval();
    m_trace->dump(sim_time);
    dut->clock = 1;
    dut->eval();
    m_trace->dump(sim_time);
  }
  int result;
  m_trace->close();
  spdlog::shutdown();

  return result;
}