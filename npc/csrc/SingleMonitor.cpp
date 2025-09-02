#include "Monitor/SingleMonitor.h"
#include "Expression/Expression.h"
#include "Simulators/RISCV32.h"
#include "spdlog/spdlog.h"
#include "utils.h"
#include <cstdint>
#include <ostream>
#include <print>
#include <regex>
#include <stdexcept>
#include <string>
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
    {
        .command = "p",
        .func = &SingleMonitor::p,
        .description = "Print infomation; p <expr>",
    }};

SingleMonitor::SingleMonitor(std::shared_ptr<RISCV32> emu, bool batch)
    : emu(emu), batch(batch) {

  if (repl.history_load("replxx_history/history.txt"))
    spdlog::info("Loaded {} history commands from {}", repl.history_size(),
                 "replxx_history/history.txt");
}

void SingleMonitor::start() {
  emu->reset();
  CommandState state = CommandState::NONE;
  bool finished = false;
  while (true) {
    if (batch)
      emu->step(-1);
    else
      state = query_command();
    if (!finished && emu->getEMUState() == RISCV32::Interrupt::EBREAK) {
      finished = true;
      if (process_trap()) {
        spdlog::info("HIT GOOD TRAP");
      } else {
        spdlog::info("HIT BAD TRAP");
      }
      continue;
    }
    if (state == CommandState::QUIT)
      break;
  }
}

bool SingleMonitor::process_trap() {
  uint32_t gpr_a0 = emu->getGPR(10);
  spdlog::info("EBREAK, a0 = {:08x}, pc = {:08x}, cycle = {}", gpr_a0,
               emu->getPC(), emu->instrCount());
  return (gpr_a0 == 0);
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
    if (ret.value() < 0) {
      println("Number of cycles must be non-negative : {}", params.front());
      return CommandState::NONE;
    }
    cnt = ret.value();
  }
  if (emu->getEMUState() != RISCV32::Interrupt::NONE) {
    println("Program has been terminated");
    return CommandState::NONE;
  }
  emu->step(cnt);
  return CommandState::NONE;
}
SingleMonitor::CommandState SingleMonitor::run(const vector<string> &params) {
  if (!params.empty()) {
    println("Too many arguments. Useage : c");
    return CommandState::NONE;
  }
  if (emu->getEMUState() != RISCV32::Interrupt::NONE) {
    println("Program has been terminated");
    return CommandState::NONE;
  }
  emu->step(-1);
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
      print("{:3} = {:08x} ", emu->gpr_names[i], emu->getGPR(i));
      if (i % 8 == 7)
        println();
    }
  } else {
    println("Useage : info {{r}}");
  }
  return CommandState::NONE;
}

SingleMonitor::CommandState
SingleMonitor::scan(const std::vector<std::string> &params) {
  if (params.size() != 2) {
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
  sz = ret.value();
  auto ret2 = to_number<long long>(params[1]);
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
    println("{:08x} : {:08x}", addr + i * 4, emu->readMemory(addr + i * 4));
  }
  return CommandState::NONE;
}

SingleMonitor::CommandState
SingleMonitor::p(const std::vector<std::string> &params) {
  string str = "";
  for (const string &s : params)
    str = str + " " + s;
  auto expr = Expression::generateExpression(str);

  if (expr.has_value()) {
    long long value;
    try {
      value = expr->eval(*emu);
      println("{}", value);
    } catch (std::logic_error e) {
      println("{}", e.what());
      println("Evaluation failed", e.what());
    }
  }
  return CommandState::NONE;
}

SingleMonitor::~SingleMonitor() { emu = nullptr; }
