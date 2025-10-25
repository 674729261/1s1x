#include "Capstone.h"
#include "Monitor/SingleMonitor.h"
#include "RingBuffer.hpp"
#include "Simulators/NEMUemu.h"
#include "Simulators/NPCemu.h"
#include "Symbols.h"
#include "spdlog/common.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstdint>
#include <exception>
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
extern "C" int pmem_read(int raddr, int clk, int valid) {
  static int clk_last = 1;
  if (clk_last == 0 && clk == 1 && valid)
    return emu->readMemory(raddr);
  return 0;
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  emu->writeMemory(waddr, wdata, wmask);
  if (mtracer) {
    println("Write to memory : {:#010x}, data : {:#010x}, mask : {:#010x}",
            (uint32_t)waddr, (uint32_t)wdata, (uint32_t)wmask);
  }
}

void register_argparse(argparse::ArgumentParser &program) {
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
  program.add_argument("-a", "--enable_audio").help("Enable audio").flag();
  program.add_argument("-v", "--enable_vga").help("Enable vga").flag();
  program.add_argument("-k", "--enable_keyboard")
      .help("Enable keyboard")
      .flag();
  program.add_argument("--itracer")
      .help("Display instruction executed")
      .default_value(16)
      .scan<'i', unsigned long>();
  program.add_argument("--mtracer").help("Display memory visited").flag();
  program.add_argument("-d", "--difftest")
      .help("Use NEMUemu as differential test")
      .flag();
  program.add_argument("--inst_ringbuffer")
      .help("Use ring buffer")
      .default_value(16)
      .scan<'i', unsigned long>();
  program.add_argument("--elf").help("ELF file path").default_value("");
}

void register_logger(argparse::ArgumentParser &program) {
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
      std::terminate();
    }
  }

  spdlog::flush_every(std::chrono::seconds(5));
  spdlog::flush_on(spdlog::level::warn);
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("NPCemu");
  register_argparse(program);
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    cerr << program;
    std::terminate();
  }
  register_logger(program);

  string image_path = program.get("--image");
  uint32_t mem_size = program.get<uint32_t>("--mem_size");
  bool batch_mode = program.get<bool>("--batch");
  unsigned long itracer = 0;
  if (program.is_used("--itracer"))
    itracer = program.get<unsigned long>("--itracer");
  mtracer = program.get<bool>("--mtracer");
  spdlog::info("Image path  : {}", image_path);
  spdlog::info("Memory size : {}", mem_size);
  bool difftest = program.get<bool>("--difftest");
  bool use_irb = program.is_used("--inst_ringbuffer");

  if (use_irb) {
    unsigned long sz_irb = program.get<unsigned long>("--inst_ringbuffer");
    InstRingBuffer::instRingBuffer.init(sz_irb);
    spdlog::info("Initialized instruction ringbuffer with size : {}", sz_irb);
  }
  bool use_ftracer = program.is_used("--elf");
  if (use_ftracer) {
    auto path_elf = program.get("--elf");
    if (path_elf.empty()) {
      println(cerr, "ELF path not specified");
      exit(1);
    }
    load_symbols(path_elf.c_str());
    spdlog::info("Initialized symbols from elf : {}", path_elf);
  }
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
      std::terminate();
    }
  }

  NPCemu::DeviceSettings device_settings;
  if (program.get<bool>("--enable_audio")) {
    spdlog::info("Audio is enabled");
    device_settings.enable_audio = true;
  }
  if (program.get<bool>("--enable_vga")) {
    spdlog::info("VGA is enabled");
    device_settings.enable_vga = true;
  }
  if (program.get<bool>("--enable_keyboard")) {
    spdlog::info("Keyboard is enabled");
    device_settings.enable_keyboard = true;
  }

  emu = make_shared<NPCemu>(mem_size, image_path, device_settings);

  SingleMonitor monitor(emu, batch_mode, itracer, mtracer, use_irb,
                        use_ftracer);
  int result;
  if (difftest)
    monitor.addRefference(nemu);
  try {
    result = monitor.start();
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    std::terminate();
  }
  emu = nullptr;
  return result;
}