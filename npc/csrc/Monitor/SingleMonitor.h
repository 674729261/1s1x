#pragma once
#include "../Simulators/RISCV32.h"
#include <memory>
#include <string>
#include <vector>
class SingleMonitor {
public:
  SingleMonitor(std::unique_ptr<RISCV32> &dut, bool batch = false);

  void start();
  ~SingleMonitor();

private:
  std::unique_ptr<RISCV32> &emu;
  bool batch;

  enum class CommandState { NONE, QUIT };

  struct CommandItem {
    std::string command;
    CommandState (SingleMonitor::*func)(const std::vector<std::string> &params);
    std::string description;
  };
  static const CommandItem command_list[];

private:
  bool process_trap();
  CommandState query_command(this SingleMonitor &self);
  CommandState help(const std::vector<std::string> &params);
  CommandState step(const std::vector<std::string> &params);
  CommandState run(const std::vector<std::string> &params);
  CommandState quit(const std::vector<std::string> &params);
  CommandState info(const std::vector<std::string> &params);
  CommandState scan(const std::vector<std::string> &params);
  CommandState p(const std::vector<std::string> &params);
};