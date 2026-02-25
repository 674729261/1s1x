
#include "DUT.h"
#include "Device/Device.h"
#include "Device/Keyboard.h"
#include "Expression/Watcher.h"
#include "Monitor.h"
#include "PerformanceCounter.h"
#include "Setup.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <Mem.h>
#include <Ref.h>
#include <Simulate.h>
#include <Vnpc_top.h>
#include <Vnpc_top___024root.h>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <format>
#include <memory>
#include <print>
#include <replxx.hxx>
#include <string>
#include <verilated.h>
#include <verilated_vcd_c.h>

long long simulation_time;
long long simulation_clocks;
long long simulation_instructions;

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

bool check_watchers() {
  bool ret = false;
  for (Expr::Expression &w : watchers) {
    auto result = w.eval();
    if (!result.value.has_value() || result.value.value() != w.last_value) {
      ret = true;
      std::string old = (w.last_value.has_value()
                             ? std::format("{:#010x}", w.last_value.value())
                             : "Error");
      std::string now = (result.value.has_value()
                             ? std::format("{:#010x}", result.value.value())
                             : "Error");
      spdlog::info("Watcher {} changed from {} to {} at PC={:#010x}", w.display,
                   old, now, dut->getPC());
    }
    w.last_value = result.value;
  }
  return ret;
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
  long long simulation_instructions_steped = 0;
  long long simulation_clocks_steped = 0;
  long long simulation_time_steped = 0;
  bool difftest_state = false;
  auto start_time = std::chrono::steady_clock::now();
  while (sim_state == SimulationState::RUNNING && steps != 0) {
    retire = false;
    dut->step_one_cycle();
    if (contextp.gotFinish()) {
      sim_state = SimulationState::HALT;
      show_trap_info();
    }
    simulation_clocks_steped++;
    if (retire) {
      steps--;
      simulation_instructions_steped++;
      bool watcher_state = check_watchers();

      if (config.difftest) {
        ref->step();
        difftest_state = check_difftest();
        if (difftest_state)
          break;
      }
      if (watcher_state)
        break;
    }
  }
  auto end_time = std::chrono::steady_clock::now();
  simulation_time_steped =
      std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                            start_time)
          .count();
  show_efficiency(simulation_clocks_steped, simulation_instructions_steped,
                  simulation_time_steped);
  simulation_instructions += simulation_instructions_steped;
  simulation_clocks += simulation_clocks_steped;
  simulation_time += simulation_time_steped;
}
void monitor_loop() {
  if (config.batch_mode) {
    run(-1);
    return;
  }
  replxx::Replxx rx;

  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path history_file = temp_dir / "replxx_history_npc.his";
  if (std::filesystem::exists(history_file)) {
    rx.history_load(history_file);
  }

  quit.store(false);
  while (!quit.load()) {
    const char *input = rx.input("(NPCemu) ");
    if (input == nullptr)
      break;
    int begin_pos = 0;
    std::string_view line_sv(input);
    while (begin_pos < line_sv.length() && std::isspace(input[begin_pos]))
      ++begin_pos;
    if (begin_pos == line_sv.length())
      continue;
    line_sv = line_sv.substr(begin_pos);

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
        rx.history_add(std::string(line_sv));
        break;
      }
    }
    if (!command_found) {
      std::println("Unknown command");
    }
  }
  rx.history_save(history_file);
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
  show_efficiency(simulation_clocks, simulation_instructions, simulation_time);
  SDL_CloseAudio();
  return result;
}