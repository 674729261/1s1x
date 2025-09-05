#include "Capstone.h"
#include "Monitor/SingleMonitor.h"
#include "Simulators/NEMUemu.h"
#include "Simulators/NPCemu.h"
#include "spdlog/common.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <print>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
using std::println, std::cerr;
using std::shared_ptr, std::make_shared;
using std::string;

shared_ptr<NPCemu> emu;
shared_ptr<NEMUemu> nemu;
bool mtracer;
extern "C" void trap(int signal) { emu->trapped = signal; }
extern "C" int pmem_read(int raddr) { return emu->readMemory(raddr); }
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  emu->writeMemory(waddr, wdata, wmask);
  if (mtracer) {
    println("Write to memory : {:#010x}, data : {:#010x}, mask : {:#010x}",
            (uint32_t)waddr, (uint32_t)wdata, (uint32_t)wmask);
  }
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
      .scan<'i', uint32_t>();
  program.add_argument("-b", "--batch").help("Batch mode").flag();
  program.add_argument("--itracer")
      .help("Display instruction executed")
      .default_value(16)
      .scan<'i', unsigned long>();
  program.add_argument("--mtracer").help("Display memory visited").flag();
  program.add_argument("-d", "--difftest")
      .help("Use NEMUemu as differential test")
      .flag();
  ;
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
  spdlog::flush_every(std::chrono::seconds(5));
  spdlog::flush_on(spdlog::level::warn);
  string image_path = program.get("--image");
  uint32_t mem_size = program.get<uint32_t>("--mem_size");
  bool batch_mode = program.get<bool>("--batch");
  unsigned long itracer = program.get<unsigned long>("--itracer");
  mtracer = program.get<bool>("--mtracer");
  spdlog::info("Image path  : {}", image_path);
  spdlog::info("Memory size : {}", mem_size);
  bool difftest = program.get<bool>("--difftest");
  if (itracer != 0) {
    if (!Capstone::capstone.load_libcapstone()) {
      spdlog::warn("Failed to initialize capstone. Ignoring itracer flag.");
      itracer = 0;
    } else {
      spdlog::info("Using capstone");
    }
  }

  if (difftest) {
    try {
      nemu = make_shared<NEMUemu>(mem_size, image_path);
    } catch (const std::exception &err) {
      println(cerr, "Load ref failed: {}", err.what());
      exit(1);
    }
  }

  emu = make_shared<NPCemu>(mem_size, image_path);
  spdlog::error("Cnt {}", (uintptr_t)emu.get());
  SingleMonitor monitor(emu, batch_mode, itracer, mtracer);
  if (difftest)
    monitor.addRefference(nemu);
  try {
    monitor.start();
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    exit(1);
  }
  emu = nullptr;
}