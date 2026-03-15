
#include "DUT.h"
#include "Flash.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <Cache.h>
#include <MROM.h>
#include <PerformanceCounter.h>
#include <Ref.h>
#include <Simulate.h>
#include <VysyxSoCFull.h>
#include <VysyxSoCFull___024root.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <nvboard.h>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>

using std::string;

static bool retire;

extern "C" void notify_retire(int32_t pc, int32_t inst) { retire = true; }

int show_trap_info() {
  if (dut->getGPR(10) == 0) {
    spdlog::info("HIT GOOD TRAP");
    return 0;
  } else {
    spdlog::warn("HIT BAD TRAP with a0 = {:010x}", dut->getGPR(10));
    return -1;
  }
}

void show_efficiency(long long clocks, long long instrs,
                     long long microseconds) {
  spdlog::info("Simulated clocks : {}", clocks);
  spdlog::info("Simulated instructions : {}", instrs);
  if (microseconds == 0 || clocks == 0)
    return;
  spdlog::info("Clocks per instruction : {:.3f}",
               static_cast<double>(clocks) / instrs);
  spdlog::info("Clocks per second : {:.3f}",
               static_cast<double>(clocks) / microseconds * 1e6);
  spdlog::info("Instructions per second : {:.3f}",
               static_cast<double>(instrs) / microseconds * 1e6);
}

bool check_difftest(Dut &dut, Ref &ref) {
  bool ret = false;
  for (int i = 0; i < 16; i++) {
    if (dut.getGPR(i) != ref.getGPR(i)) {
      spdlog::error("gpr {} differs from ref : should be {:08x}, got {:08x}",
                    gpr_names[i], ref.getGPR(i), dut.getGPR(i));
      ret = true;
    }
  }
  if (dut.getPC() != ref.getPC()) {
    spdlog::error("PC differs from ref : should be {:08x}, got {:08x}",
                  ref.getPC(), dut.getPC());
    ret = true;
  }

  return ret;
}

void run(unsigned long long steps) {
  if (sim_state == SimulationState::HALT) {
    spdlog::info("Program has hit trap @ PC = {:#010x}, a0 = {:#010x}",
                 dut->getPC(), dut->getGPR(10));
    return;
  }
  if (sim_state == SimulationState::DIFFTEST_FAILED) {
    check_difftest(*dut, *ref);
    return;
  }
  long long simulation_instructions_steped = 0;
  long long simulation_clocks_steped = 0;
  long long simulation_time_steped = 0;
  auto start_time = std::chrono::steady_clock::now();
  unsigned long long cur_step = 0;
  while (sim_state == SimulationState::RUNNING && steps != cur_step) {
    retire = false;
    dut->step_one_cycle();
    if (contextp->gotFinish()) {
      sim_state = SimulationState::HALT;
      show_trap_info();
    }
    simulation_clocks_steped++;
    if (retire) {
      cur_step++;
      simulation_instructions_steped++;
      check_difftest(*dut, *ref);
    }
  }
  auto end_time = std::chrono::steady_clock::now();
  simulation_time_steped =
      std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                            start_time)
          .count();
  if (!config.batch_mode)
    show_efficiency(simulation_clocks_steped, simulation_instructions_steped,
                    simulation_time_steped);
  performance_statistics.simulation_instructions +=
      simulation_instructions_steped;
  performance_statistics.simulation_clocks += simulation_clocks_steped;
  performance_statistics.simulation_time += simulation_time_steped;
}

int simulate() {
  // init_mrom(config.image_path);
  init_flash(config.image_path);
  // Verilated::commandArgs(argc, argv);
  std::unique_ptr<VerilatedContext> contextp =
      std::make_unique<VerilatedContext>();
  // contextp->commandArgs(argc, argv);

  Verilated::traceEverOn(true);

  dut = std::make_unique<Dut>(config, contextp.get());
  ref = std::make_unique<Ref>(config, *dut, 3, 1);
  if (config.nvboard) {
    dut->nvboard_bind();
    nvboard_init();
  }
  ref->reset(*dut);
  dut->reset();
  clear_performance_count();
  bool difftest_state = false;
  auto start_time = std::chrono::steady_clock::now();
  while (!contextp->gotFinish()) {
    retire = false;
    if (dut->getResetCore()) {
      ref->reset(*dut);
      ref->sync_state();
      clear_performance_count();
    }
    if (config.nvboard)
      nvboard_update();
    clock_count++;
    dut->step_one_cycle();

    if (retire) {
      // std::println("{:08x}", dut->getPC());
      inst_count++;
      if (config.difftest) {
        ref->step();
        difftest_state = check_difftest(*dut, *ref);
        if (difftest_state)
          break;
      }
    }
  }
  auto end_time = std::chrono::steady_clock::now();

  int result;
  if (contextp->gotFinish()) {
    if (dut->getGPR(10) == 0) {
      spdlog::info("HIT GOOD TRAP");
      result = 0;
    } else {
      spdlog::warn("HIT BAD TRAP with a0 = {:010x}", dut->getGPR(10));
      result = -1;
    }
  } else if (difftest_state) {
    spdlog::warn("DIFFTEST FAILED");
    result = -2;
  } else {
    spdlog::warn("FAILED TO HALT");
    result = -3;
  }
  dut->print_all_gpr();
  spdlog::info("Reference cache hit count : {}", ref->getCacheHit());
  spdlog::info("Reference instruction count : {}", ref->instrCount());

  display_performance(start_time, end_time);
  dut = nullptr;
  return result;
}

int main(int argc, char *argv[]) {
  config = process_args(argc, argv);
  int return_value = -1;
  try {
    return_value = simulate();
  } catch (std::exception e) {
    std::println(std::cerr, "Error : {}", e.what());
  }
  spdlog::shutdown();
  return return_value;
}