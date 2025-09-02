#include "Monitor/SingleMonitor.h"
#include "Simulators/NPCemu.h"
#include "Simulators/RISCV32.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

using std::println, std::cerr, std::clog;
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
      .default_value(string(""));
  program.add_argument("-l", "--log")
      .help("The program image file")
      .required()
      .implicit_value("std::any value");
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
  string image_path = program.get("--image");
  int mem_size = program.get<int>("--mem_size");
  bool batch_mode = program.get<bool>("--batch");
  println(clog, "Image path  : {}", image_path);
  println(clog, "Memory size : {}", mem_size);

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