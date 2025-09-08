#pragma once
#include "../Expression/Expression.h"
#include "../Simulators/RISCV32.h"
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <replxx.hxx>
#include <string>
#include <utility>
#include <vector>
class SingleMonitor {
public:
  SingleMonitor(std::shared_ptr<RISCV32> dut, bool batch = false,
                unsigned long itracer = 16, bool mtracer = false,
                bool irb = false, bool ftracer = false);

  void start();
  void addRefference(std::shared_ptr<RISCV32> ref);
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
  std::vector<std::shared_ptr<RISCV32>> emus;
  unsigned long itracer;
  bool batch, mtracer, irb, ftracer;

  enum CommandState {
    NONE = 0, // OK to read next command
    QUIT = 1  // End the monitor
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
  std::pair<int, int> check_diff();
  std::pair<int, int> diff_fault{-1, -1};
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
