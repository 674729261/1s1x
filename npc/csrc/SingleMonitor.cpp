#include "Monitor/SingleMonitor.h"
#include "Simulators/RISCV32.h"
#include <algorithm>
#include <charconv>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <print>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

using std::cin, std::getline;
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
     .description = "Check registers"}};

SingleMonitor::SingleMonitor(std::unique_ptr<RISCV32> &emu, bool batch)
    : emu(emu), batch(batch) {}

void SingleMonitor::start() {
  emu->reset();
  CommandState state = CommandState::NONE;
  while (true) {
    if (batch)
      emu->step(-1);
    else
      state = query_command();
    if (emu->getEMUState() == RISCV32::Interrupt::EBREAK) {
      if (process_trap()) {
        println(std::clog, "HIT GOOD TRAP");
      } else {
        println(std::clog, "HIT BAD TRAP");
      }
      break;
    }
    if (state == CommandState::QUIT)
      break;
  }
}

bool SingleMonitor::process_trap() {
  uint32_t gpr_a0 = emu->getGPR(10);
  println(std::cerr, "EBREAK, a0 = {:08x}, pc = {:08x}, cycle = {}", gpr_a0,
          emu->getPC(), emu->instrCount());
  return (gpr_a0 == 0);
}

SingleMonitor::CommandState
SingleMonitor::query_command(this SingleMonitor &self) {
  print("(NPCemu)");
  string command;
  getline(cin, command);
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
    std::string_view p = params.front();
    auto [ptr, ec] = std::from_chars(p.begin(), p.end(), cnt);
    if (ec == std::errc::result_out_of_range) {
      println("Argument is too large : {}", p);
      return CommandState::NONE;
    } else if (ptr != p.end() || ec != std::errc()) {
      println("Invalid argument : {}", p);
      return CommandState::NONE;
    } else if (cnt < 0) {
      println("Number of cycles must be non-negative : {}", p);
      return CommandState::NONE;
    }
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
      print("{:4} = {:08x} ", emu->gpr_names[i], emu->getGPR(i));
      if (i % 8 == 7)
        println();
    }
  } else {
    println("Useage : info {{r}}");
  }
  return CommandState::NONE;
}
SingleMonitor::~SingleMonitor() { emu = nullptr; }
