#pragma once
#include "../Simulators/RISCV32.h"
#include <functional>
#include <memory>
#include <string>
class SingleMonitor {
public:
  SingleMonitor(std::unique_ptr<RISCV32> dut, unsigned int max_cycles,
                bool batch = false);

  void start();

private:
  unsigned int max_cycles;
  std::unique_ptr<RISCV32> dut;
  bool batch;

  struct CommandItem {
    std::string command;
    int (SingleMonitor::*func)();
    std::string description;
  };
  const CommandItem command_list[3] = {
      {.command = "help",
       .func = &SingleMonitor::help,
       .description = "Show descriptions of all commands"},
      {.command = "s",
       .func = &SingleMonitor::step,
       .description = "Step several cycles; s [cnt=1]"},
      {.command = "c",
       .func = &SingleMonitor::run,
       .description = "Continue the program"}};

private:
  int help();
  int step();
  int run();

  void query_command();
};