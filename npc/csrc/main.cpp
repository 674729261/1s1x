#include "Monitor/SingleMonitor.h"
#include "Simulators/NPCemu.h"
#include "Simulators/RISCV32.h"
#include "spdlog/common.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
using std::println, std::cerr;
using std::string;
using std::unique_ptr, std::make_unique;

NPCemu *emu_cpy;

extern "C" void trap(int signal) { emu_cpy->trapped = signal; }
extern "C" int pmem_read(int raddr) { return emu_cpy->readMemory(raddr); }
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  emu_cpy->writeMemory(waddr, wdata, wmask);
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("NPCemu");

  program.add_argument("-i", "--image")
      .help("Path to log file")
      .nargs(1)
      .required();
  program.add_argument("-l", "--log").help("The program image file");
  program.add_argument("-z", "--mem_size")
      .help("Size of memory")
      .default_value(1 << 24)
      .scan<'i', int>();
  program.add_argument("-b", "--batch").help("Batch mode").flag();
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    cerr << program;
    exit(1);
  }
  bool provided_logfile = program.is_used("--log");
  if (provided_logfile) {
    try {
      string log_path = program.get("--log");
      auto console_sink =
          std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

      auto file_sink =
          std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, true);
      file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
      spdlog::logger logger("multi_logger", {console_sink, file_sink});
      spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
      spdlog::info("Logging to file : {}", log_path);
    } catch (const spdlog::spdlog_ex &e) {
      println(cerr, "Log init failed: {}", e.what());
      exit(1);
    }
  }
  spdlog::flush_every(std::chrono::seconds(3));
  spdlog::flush_on(spdlog::level::warn);
  string image_path = program.get("--image");
  int mem_size = program.get<int>("--mem_size");
  bool batch_mode = program.get<bool>("--batch");
  spdlog::info("Image path  : {}", image_path);
  spdlog::info("Memory size : {}", mem_size);

  unique_ptr<RISCV32> emu = make_unique<NPCemu>(mem_size, image_path);
  emu_cpy = dynamic_cast<NPCemu *>(emu.get());
  SingleMonitor monitor(emu, batch_mode);
  try {
    monitor.start();
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    exit(1);
  }
  emu = nullptr;
}