#include "Monitor/SingleMonitor.h"
#include "ELFParser.h"
#include "Expression/Expression.h"
#include "RingBuffer.hpp"
#include "Simulators/RISCV32.h"
#include "spdlog/spdlog.h"
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
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using std::print, std::println;
using std::regex, std::sregex_token_iterator;
using std::string;
using std::vector;

const SingleMonitor::CommandItem SingleMonitor::command_list[] = {
    {.command = "help",
     .func = &SingleMonitor::help,
     .description = "Show descriptions of all commands"},
    {.command = "s",
     .func = &SingleMonitor::step,
     .description = "Step several cycles; s [cnt=1]"},
    {.command = "c",
     .func = &SingleMonitor::run,
     .description = "Continue the program"},
    {.command = "q",
     .func = &SingleMonitor::quit,
     .description = "Quit the simulation"},
    {.command = "info",
     .func = &SingleMonitor::info,
     .description = "Check registers"},
    {.command = "x",
     .func = &SingleMonitor::scan,
     .description = "Scan memory; x [size] [addr]"},
    {.command = "p",
     .func = &SingleMonitor::p,
     .description = "Print infomation; p <expr>"},
    {.command = "w",
     .func = &SingleMonitor::w,
     .description = "Setup a watcher; w <expr>"},
    {.command = "d",
     .func = &SingleMonitor::d,
     .description = "Remove a watcher; d [index]"},
};

SingleMonitor::SingleMonitor(std::shared_ptr<RISCV32> emu, bool batch,
                             unsigned long itracer, bool mtracer, bool irb,
                             bool ftracer, std::string_view elf_path)
    : batch(batch), itracer(itracer), mtracer(mtracer), irb(irb),
      ftracer(ftracer) {
  emus.push_back(emu);

  repl.set_max_history_size(64);
  auto tmp_path =
      std::filesystem::temp_directory_path().append("NPCemu_history.txt");
  if (repl.history_load(tmp_path))
    spdlog::info("Loaded {} history commands from {}", repl.history_size(),
                 tmp_path.string());

  if (ftracer) {
    symbol_table = std::make_shared<ProgSymTab>();
    symbol_table->init_and_parse(elf_path);
  }
}
void SingleMonitor::addRefference(std::shared_ptr<RISCV32> ref) {
  emus.push_back(ref);
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
        emus.front()->pause(false);
        auto n_inst = emus.front()->instrCount();
        auto start = steady_clock::now();
        emus.front()->simulate(-1);
        auto end = steady_clock::now();
        n_inst = emus.front()->instrCount() - n_inst;
        double elapsed = duration_cast<nanoseconds>(end - start).count();
        spdlog::info("Average speed : {:.1f} inst/s",
                     1'000'000'000.0 * n_inst / elapsed);

        emus.front()->pause(true);
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
  if (itracer)
    InstRingBuffer::instRingBuffer.display();
  return state;
}

bool SingleMonitor::process_trap() {
  uint32_t gpr_a0 = emus.front()->getGPR(10);
  spdlog::info("EBREAK, a0 = {:08x}, pc = {:08x}, cycle = {}", gpr_a0,
               emus.front()->getPC(), emus.front()->instrCount());
  return (gpr_a0 == 0);
}

void SingleMonitor::simulate(unsigned long cnt) {
  using namespace std::chrono;
  int n_inst = emus.front()->instrCount();
  auto start = steady_clock::now();
  unsigned long max_display_inst = cnt < 0 ? 0 : itracer;
  max_display_inst = std::min(max_display_inst, cnt);

  bool triggered = false;
  for (auto &e : emus) {
    e->pause(false);
  }
  while (cnt--) {
    if (max_display_inst > 0) {
      for (auto &e : emus) {
        e->step(itracer, irb, symbol_table);
      }
      max_display_inst--;
    } else
      for (auto &e : emus)
        e->step(false, irb, symbol_table);
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
        for (auto &e : emus) {
          e->pause(true);
        }
        return;
      }
    }
    if (diff_fault.first >= 0 || triggered ||
        emus.front()->getEMUState() != RISCV32::Interrupt::NONE)
      break;
  }
  for (auto &e : emus) {
    e->pause(true);
  }
  auto end = steady_clock::now();
  n_inst = emus.front()->instrCount() - n_inst;
  double elapsed = duration_cast<nanoseconds>(end - start).count();
  spdlog::info("Simulated {} cycles, PC is now {:#010x}", n_inst,
               emus.front()->getPC());
  spdlog::info("Average speed : {:.1f} inst/s",
               1'000'000'000.0 * n_inst / elapsed);
}

SingleMonitor::CommandState
SingleMonitor::query_command(this SingleMonitor &self) {
  string command = self.repl.input("(NPCemu)");
  self.repl.history_add(command);
  regex del(R"(\s+)");

  sregex_token_iterator it(command.begin(), command.end(), del, -1);
  sregex_token_iterator end;
  vector<string> params;
  string head;
  while (it != end) {
    if (head.empty())
      head = *it;
    else
      params.push_back(*it);
    it++;
  }
  if (!head.empty()) {
    for (const auto &item : command_list) {
      if (item.command == head) {
        return (self.*item.func)(params);
      }
    }
    println("Invalid command : {}", head);
  }
  return CommandState::NONE;
}

SingleMonitor::CommandState SingleMonitor::help(const vector<string> &params) {
  for (const auto &item : command_list) {
    println("{:<10} - {}", item.command, item.description);
  }
  return CommandState::NONE;
}

SingleMonitor::CommandState SingleMonitor::step(const vector<string> &params) {
  if (params.size() > 1) {
    println("Too many arguments. Useage : s [cnt=1]");
    return CommandState::NONE;
  }
  int cnt = 1;
  if (params.size() == 1) {
    auto ret = to_number<int>(params.front());
    if (!ret.has_value())
      return CommandState::NONE;
    if (ret.value() <= 0) {
      println("Number of cycles must be positive : {}", params.front());
      return CommandState::NONE;
    }
    cnt = ret.value();
  }
  if (emus.front()->getEMUState() != RISCV32::Interrupt::NONE) {
    println("Program has been terminated");
    return CommandState::NONE;
  }
  simulate(cnt);
  return CommandState::NONE;
}
SingleMonitor::CommandState SingleMonitor::run(const vector<string> &params) {
  if (!params.empty()) {
    println("Too many arguments. Useage : c");
    return CommandState::NONE;
  }
  if (emus.front()->getEMUState() != RISCV32::Interrupt::NONE) {
    println("Program has been terminated");
    return CommandState::NONE;
  }
  simulate(-1);
  return CommandState::NONE;
}

SingleMonitor::CommandState SingleMonitor::quit(const vector<string> &params) {
  if (!params.empty()) {
    println("Too many arguments. Useage : q");
    return CommandState::NONE;
  }
  return CommandState::QUIT;
}
SingleMonitor::CommandState
SingleMonitor::info(const std::vector<std::string> &params) {
  if (params.size() != 1) {
    println("Useage : info {{r}}");
    return CommandState::NONE;
  }
  if (params.front() == "r") {
    for (int i = 0; i < 32; i++) {
      print("{:3} = {:08x} ", emus.front()->gpr_names[i],
            emus.front()->getGPR(i));
      if (i % 8 == 7)
        println();
    }
  } else if (params.front() == "w") {
    println("{:-^50}", "");
    for (const auto &wat : watchers) {
      println("{:>5}|{:>#10x}|{:<}", wat.id, wat.last,
              wat.expression.stringify());
    }
    println("{:-^50}", "");
    println("{} watcher(s)", watchers.size());
  } else
    println("Useage : info {{r}}");
  return CommandState::NONE;
}

SingleMonitor::CommandState
SingleMonitor::scan(const std::vector<std::string> &params) {
  if (params.size() < 2) {
    println("Useage : x [size] [addr]");
    return CommandState::NONE;
  }
  int sz;
  uint32_t addr;
  auto ret = to_number<int>(params.front());
  if (!ret.has_value())
    return CommandState::NONE;
  if (ret.value() < 0) {
    println("Size must be non-negative : {}", params.front());
    return CommandState::NONE;
  }
  string str = "";
  for (int i = 1; i < params.size(); i++)
    str = str + " " + params[i];
  sz = ret.value();
  auto ret2 = Expression::evalExpression(*emus.front(), str);
  if (!ret2.has_value())
    return CommandState::NONE;
  if (ret2.value() < 0 || ret2.value() >= UINT32_MAX) {
    println("Address must be non-negative and less than {:x} : {} ", UINT32_MAX,
            params[1]);
    return CommandState::NONE;
  }
  addr = ret2.value();
  addr &= ~0x3;
  for (int i = 0; i < sz; i++) {
    println("{:08x} : {:08x}", addr + i * 4,
            emus.front()->readMemory(addr + i * 4));
  }
  return CommandState::NONE;
}

SingleMonitor::CommandState
SingleMonitor::p(const std::vector<std::string> &params) {
  string str = "";
  for (const string &s : params)
    str = str + " " + s;
  auto result = Expression::evalExpression(*emus.front(), str);
  if (result.has_value())
    println("{:#010x}", result.value());
  return CommandState::NONE;
}

SingleMonitor::CommandState
SingleMonitor::w(const std::vector<std::string> &params) {
  static int id = 0;
  string str = "";
  for (const string &s : params)
    str = str + " " + s;
  auto result = Watcher::generateWatcher(*emus.front(), str);

  if (result.has_value()) {
    watchers.push_back(std::move(result.value()));
    watchers.back().id = id++;
    println("Setup watcher #{}, now = {:#010x}", watchers.size() - 1,
            result.value().last);
  }
  return CommandState::NONE;
}
SingleMonitor::CommandState
SingleMonitor::d(const std::vector<std::string> &params) {
  if (params.size() < 1) {
    println("Useage : d [index]");
    return CommandState::NONE;
  }
  auto id = to_number<int>(params.front());
  if (!id.has_value()) {
    println("Invalid index : {} ", params.front());
    return CommandState::NONE;
  }
  for (auto iter = watchers.begin(); iter != watchers.end(); iter++) {
    if (iter->id == id.value()) {
      watchers.erase(iter);
      return CommandState::NONE;
    }
  }
  println("Invalid index : {} ", params.front());
  return CommandState::NONE;
}

SingleMonitor::~SingleMonitor() {
  auto tmp_path =
      std::filesystem::temp_directory_path().append("NPCemu_history.txt");
  repl.history_save(tmp_path);
}

std::optional<SingleMonitor::Watcher>
SingleMonitor::Watcher::generateWatcher(RISCV32 &dut, std::string_view expr) {
  auto e = Expression::generateExpression(expr);
  uint32_t value;
  if (!e.has_value())
    return std::nullopt;

  try {
    value = e->eval(dut);
  } catch (std::logic_error e) {
    println("{}", e.what());
    println("Evaluation failed", e.what());
    return std::nullopt;
  }
  SingleMonitor::Watcher ret{std::move(e.value()), value};
  return ret;
}

std::pair<int, int> SingleMonitor::check_diff() {

  for (int i = 1; i < emus.size(); i++) {
    for (int j = 0; j < 33; j++) {
      if (emus.front()->getGPR(j) != emus[i]->getGPR(j)) {
        return {i, j};
      }
    }
  }
  return {-1, -1};
}