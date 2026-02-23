
#include "DUT.h"
#include "Device/Device.h"
#include "Device/Keyboard.h"
#include "Setup.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <Mem.h>
#include <Ref.h>
#include <Simulate.h>
#include <Vnpc_top.h>
#include <Vnpc_top___024root.h>
#include <chrono>
#include <memory>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>

double simulation_time;
long long simulation_clocks;
long long simulation_instructions;

static bool retire;

extern "C" void notify_retire(int32_t pc, int32_t inst) { retire = true; }

bool check_difftest() {
  bool ret = false;
  for (int i = 0; i < 16; i++) {
    if (dut->getGPR(i) != ref->cpu.gpr[i]) {
      spdlog::error("gpr {} differs from ref : should be {:08x}, got {:08x}",
                    gpr_names[i], ref->cpu.gpr[i], dut->getGPR(i));
      ret = true;
    }
  }
  if (dut->getPC() != ref->cpu.pc) {
    spdlog::error("PC differs from ref : should be {:08x}, got {:08x}",
                  ref->cpu.pc, dut->getPC());
    ret = true;
  }

  return ret;
}

void monitor_loop() {
  bool difftest_state = false;
  auto start_time = std::chrono::steady_clock::now();
  while (sim_state == SimulationState::RUNNING) {
    retire = false;
    if (config.difftest && dut->top->rootp->npc_top__DOT__reset) {
      ref->reset(*dut);
      ref->sync_state();
    }
    // std::println("PC = {:08x}", dut->getPC());
    dut->step_one_cycle();
    if (contextp.gotFinish())
      sim_state = SimulationState::HALT;
    if (retire) {
      // std::println("PC = {:08x}", dut->getPC());
      // std::println("{:08x}", ref.getPC());
      if (config.difftest) {
        ref->step();
        difftest_state = check_difftest();
        if (difftest_state)
          break;
      }
    }
  }
  auto end_time = std::chrono::steady_clock::now();
}

int simulate(int argc, char *argv[]) {
  // init_mrom(config.image_path);
  init_mem(config.image_path);
  RTC_init();
  audio_init();
  Verilated::commandArgs(argc, argv);
  contextp.commandArgs(argc, argv);
  Verilated::traceEverOn(config.use_waveform);
  if (config.enable_vga) {
    vga_init();
    keyboard_init();
    init_vga_kbd_thread();
  }
  dut = std::make_unique<Dut>(&contextp);
  ref = std::make_unique<Ref>(*dut);
  ref->reset(*dut);
  dut->reset();
  monitor_loop();
  int result;
  if (sim_state == SimulationState::HALT) {
    if (dut->getGPR(10) == 0) {
      spdlog::info("HIT GOOD TRAP");
      result = 0;
    } else {
      spdlog::warn("HIT BAD TRAP with a0 = {:010x}", dut->getGPR(10));
      result = -1;
    }
  } else if (sim_state == SimulationState::DIFFTEST_FAILED) {
    spdlog::warn("DIFFTEST FAILED");
    result = -2;
  } else {
    spdlog::warn("DID NOT HALT");
    result = -3;
  }
  dut->print_all_gpr();
  if (device_thread)
    device_thread->request_stop();

  SDL_CloseAudio();
  return result;
}