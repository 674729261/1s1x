
#include "DUT.h"
#include "Device/Device.h"
#include "Device/Keyboard.h"
#include "Monitor.h"
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
#include <replxx.hxx>
#include <verilated.h>
#include <verilated_vcd_c.h>

long long simulation_time;
long long simulation_clocks;
long long simulation_instructions;

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
void run(unsigned long long steps) {
  if (sim_state == SimulationState::HALT) {
    spdlog::info("Program has hit trap @ PC = {:#010x}, a0 = {:#010x}",
                 dut->getPC(), dut->getGPR(10));
    return;
  }
  if (sim_state == SimulationState::DIFFTEST_FAILED) {
    check_difftest();
    return;
  }
  bool difftest_state = false;
  auto start_time = std::chrono::steady_clock::now();
  while (sim_state == SimulationState::RUNNING && steps != 0) {
    retire = false;
    dut->step_one_cycle();
    if (contextp.gotFinish()) {
      sim_state = SimulationState::HALT;
      show_trap_info();
    }
    if (retire) {
      steps--;
      simulation_instructions++;
      if (config.difftest) {
        ref->step();
        difftest_state = check_difftest();
        if (difftest_state)
          break;
      }
    }
  }
  auto end_time = std::chrono::steady_clock::now();
  simulation_time += std::chrono::duration_cast<std::chrono::microseconds>(
                         start_time - end_time)
                         .count();
}
void monitor_loop() {
  replxx::Replxx rx;
  quit.store(false);
  while (!quit.load()) {
    const char *input = rx.input("(NPCemu) ");
    if (input == nullptr)
      break;
    std::string line(input);
    std::string_view line_sv(line);
    bool command_found = false;
    for (const auto &item : cmd_list) {
      if (line_sv.starts_with(item.command)) {
        std::string_view arg_sv = line_sv.substr(item.command.length());
        CmdResult ret = item.call(std::string(arg_sv));
        if (ret == CmdResult::INVALID_ARG) {
          std::println("Invalid argument.");
          std::println("{}", item.help);
        }
        command_found = true;
        break;
      }
    }
    if (!command_found) {
      std::println("Unknown command");
    }
  }
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
    result = show_trap_info();
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