#pragma once
#include "../Expression/Expression.h"
#include "../Simulators/RISCV32.h"
#include "Device/Device.h"
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <replxx.hxx>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
class SingleMonitor {
public:
  SingleMonitor(std::shared_ptr<RISCV32> dut, size_t MemSize,
                std::string_view program, Devices::DeviceSettings ds,
                bool batch = false, unsigned long itracer = 16,
                bool mtracer = false, unsigned long irb = 16,
                bool ftracer = false, std::string_view elf_path = "");

  int start();
  void addReference(std::shared_ptr<RISCV32> ref);
  ~SingleMonitor();

  class Watcher {
  public:
    static std::optional<Watcher>
    generateWatcher(RISCV32 &dut, Devices &devices, std::string_view expr);
    Expression expression;
    uint32_t last;
    int id;
  };

private:
  std::vector<std::shared_ptr<RISCV32>> emus;
  std::shared_ptr<Devices> devices;
  unsigned long itracer, irb;
  bool batch, mtracer;

  enum CommandState {
    QUIT = 0, // End the monitor
    NONE = 1, // OK to read next command
    BAD_TRAP
  };

  std::list<Watcher> watchers;

  struct CommandItem {
    std::string command;
    CommandState (SingleMonitor::*func)(const std::vector<std::string> &params);
    std::string description;
  };
  static const CommandItem command_list[];
  replxx::Replxx repl;

  std::shared_ptr<Tracer> tracer;

private:
  bool process_trap();
  std::pair<int, int> check_diff();
  std::pair<int, int> diff_fault{-1, -1};
  void simulate(unsigned long long cnt);

  void check_device();
  bool check_watchers();
  void flush_inst(RISCV32 &e);

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
