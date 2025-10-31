#include "Device/Device.h"
#include <Expression/Expression.h>
#include <Monitor/SingleMonitor.h>
#include <Simulators/RISCV32.h>
#include <Tracer/Tracer.h>
#include <chrono>
#include <cstdint>
#include <endian.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <my_utils.h>
#include <ostream>
#include <print>
#include <regex>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using std::println;
using std::regex, std::sregex_token_iterator;
using std::string;

SingleMonitor::SingleMonitor(std::shared_ptr<RISCV32> emu,
                             Devices::DeviceSettings ds, bool batch,
                             unsigned long itracer, bool mtracer,
                             unsigned long irb, bool ftracer,
                             std::string_view elf_path)
    : batch(batch), itracer(itracer), mtracer(mtracer), irb(irb) {
  emus.push_back(emu);

  repl.set_max_history_size(64);
  auto tmp_path =
      std::filesystem::temp_directory_path().append("NPCemu_history.txt");
  if (repl.history_load(tmp_path))
    spdlog::info("Loaded {} history commands from {}", repl.history_size(),
                 tmp_path.string());

  tracer = std::make_shared<Tracer>(ftracer, elf_path, irb);
  emus.front()->tie_tracer(tracer);

  devices = std::make_shared<Devices>(ds);

  devices->init_ioe();
  emu->tie_devices(devices);
}

void SingleMonitor::addReference(std::shared_ptr<RISCV32> ref) {
  emus.push_back(ref);
  ref->tie_devices(devices);
}

int SingleMonitor::start() {
  using namespace std::chrono;
  for (auto &e : emus)
    e->reset();
  CommandState state = CommandState::NONE;
  bool finished = false;
  try {
    while (true) {
      if (batch) {
        tracer->set_display(false);
        devices->pause(false);
        auto n_inst = emus.front()->instrCount();
        // emus.front()->simulate(-1);
        auto &main_emu = *emus.front();
        auto start = steady_clock::now();
        while (main_emu.getEMUState() == RISCV32::Interrupt::NONE &&
               !devices->is_quit()) {
          main_emu.step();
        }

        auto end = steady_clock::now();
        n_inst = emus.front()->instrCount() - n_inst;
        double elapsed = duration_cast<nanoseconds>(end - start).count();
        spdlog::info("Average speed : {:.1f} inst/s",
                     1'000'000'000.0 * n_inst / elapsed);
        if (devices->is_quit())
          state = CommandState::QUIT;
        devices->pause(true);
      } else
        state = query_command();
      if (!finished &&
          emus.front()->getEMUState() == RISCV32::Interrupt::EBREAK) {
        finished = true;
        if (process_trap()) {
          spdlog::info("HIT GOOD TRAP");
          state = CommandState::QUIT;
        } else {
          spdlog::info("HIT BAD TRAP");
          state = CommandState::BAD_TRAP;
        }
      }
      if (diff_fault.first >= 0) {
        spdlog::info("Reg {} differs with ref #{} @ PC = {:#010x}\nShould be "
                     "{:#010x}, got {:#010x}",
                     RISCV32::gpr_names[diff_fault.second], diff_fault.first,
                     emus.front()->getPC(),
                     emus[diff_fault.first]->getGPR(diff_fault.second),
                     emus.front()->getGPR(diff_fault.second));
        diff_fault = {-1, -1};
        if (batch)
          state = CommandState::BAD_TRAP;
      }
      if (state != CommandState::NONE)
        break;
    }
  } catch (const std::logic_error &e) {
    std::println(std::cerr, "{}", e.what());
  }
  if (irb > 0)
    tracer->show_history_instructions();
  return state;
}

bool SingleMonitor::process_trap() {
  uint32_t gpr_a0 = emus.front()->getGPR(10);
  spdlog::info("EBREAK, a0 = {:08x}, pc = {:08x}, cycle = {}", gpr_a0,
               emus.front()->getPC(), emus.front()->instrCount());
  return (gpr_a0 == 0);
}

void SingleMonitor::simulate(unsigned long long cnt) {
  using namespace std::chrono;
  devices->reset_quit();
  int n_inst = emus.front()->instrCount();
  auto start = steady_clock::now();
  unsigned long long max_display_inst = itracer;
  max_display_inst = std::min(max_display_inst, cnt);

  bool triggered = false;
  devices->pause(false);

  tracer->set_display(max_display_inst > 0);

  while (cnt--) {
    if (max_display_inst > 0) [[unlikely]] {
      for (auto &e : emus) {
        e->step();
      }
      max_display_inst--;
      if (max_display_inst == 0)
        tracer->set_display(false);
    } else {
      for (auto &e : emus)
        e->step();
    }

    if (emus.size() > 1)
      diff_fault = check_diff();

    for (auto &wat : watchers) {
      try {
        uint32_t value = wat.expression.eval(*emus.front());
        if (value != wat.last) {
          println("Watcher #{}@{:#010x} : {}", wat.id, emus.front()->getPC(),
                  wat.expression.stringify());
          println("{:#010x} -> {:#010x}", wat.last, value);
          wat.last = value;
          triggered = true;
        }
      } catch (std::logic_error e) {
        spdlog::warn(
            "Error encountered while evaluating watcher #{}@{:#010x} : {}, "
            "error info : {}",
            wat.id, emus.front()->getPC(), wat.expression.stringify(),
            e.what());
        devices->pause(true);
        return;
      }
    }
    if (diff_fault.first >= 0 || triggered || devices->is_quit() ||
        emus.front()->getEMUState() != RISCV32::Interrupt::NONE)
      break;
  }
  devices->pause(true);
  auto end = steady_clock::now();
  n_inst = emus.front()->instrCount() - n_inst;
  double elapsed = duration_cast<nanoseconds>(end - start).count();
  spdlog::info("Simulated {} cycles, PC is now {:#010x}", n_inst,
               emus.front()->getPC());
  spdlog::info("Average speed : {:.1f} inst/s",
               1'000'000'000.0 * n_inst / elapsed);
}
