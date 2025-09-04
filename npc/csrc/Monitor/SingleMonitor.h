#pragma once
#include "../Expression/Expression.h"
#include "../Simulators/RISCV32.h"
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <replxx.hxx>
#include <string>
class SingleMonitor {
public:
  SingleMonitor(std::shared_ptr<RISCV32> dut, bool batch = false,
                bool itracer = false, bool mtracer = false);

  void start();
  ~SingleMonitor();

  class Watcher {
  public:
    static std::optional<Watcher> generateWatcher(RISCV32 &dut,
                                                  std::string_view expr);
    Expression expression;
    uint32_t last;
    int id;
  };

private:
  std::shared_ptr<RISCV32> emu;
  bool batch, itracer, mtracer;

  enum class CommandState {
    NONE, // OK to read next command
    QUIT  // End the monitor
  };

  std::list<Watcher> watchers;

  struct CommandItem {
    std::string command;
    CommandState (SingleMonitor::*func)(const std::vector<std::string> &params);
    std::string description;
  };
  static const CommandItem command_list[];
  replxx::Replxx repl;

private:
  bool process_trap();

  void simulate(unsigned long cnt);

  CommandState query_command(this SingleMonitor &self);
  CommandState help(const std::vector<std::string> &params);
  CommandState step(const std::vector<std::string> &params);
  CommandState run(const std::vector<std::string> &params);
  CommandState quit(const std::vector<std::string> &params);
  CommandState info(const std::vector<std::string> &params);
  CommandState scan(const std::vector<std::string> &params);
  CommandState p(const std::vector<std::string> &params);
  CommandState w(const std::vector<std::string> &params);
  CommandState d(const std::vector<std::string> &params);
};
