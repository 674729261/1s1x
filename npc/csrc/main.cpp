#include "Simulators/NPCemu.h"
#include "Simulators/RISCV32.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

using std::println, std::cerr, std::clog;
using std::string;
using std::unique_ptr, std::make_unique;

unique_ptr<NPCemu> emu;

extern "C" void trap(int signal) { emu->trapped = 1; }
extern "C" int pmem_read(int raddr) { return emu->readMemory(raddr); }
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  emu->writeMemory(waddr, wdata, wmask);
}

bool process_trap() {
  uint32_t gpr_a0 = emu->getGPR(10);
  println(cerr, "EBREAK, a0 = {:08x}, pc = {:08x}, cycle = {}", gpr_a0,
          emu->getPC(), emu->instrCount());
  return (gpr_a0 == 0);
}

void simulate(std::size_t max_cycles) {
  try {
    emu.reset();
    while (true) {
      auto state = emu->step(max_cycles);
      if (state == RISCV32::Interrupt::EBREAK) {
        if (process_trap()) {
          println(clog, "HIT GOOD TRAP");
        } else {
          println(clog, "HIT BAD TRAP");
        }
        break;
      }
    }
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("NPCemu");

  program.add_argument("-i", "--image")
      .help("The program image file")
      .required();
  program.add_argument("-c", "--cycles")
      .help("Maximum clock cycles")
      .default_value(-1)
      .scan<'d', int>();
  program.add_argument("-z", "--mem_size")
      .help("Size of memory(hex)")
      .default_value(1 << 24)
      .scan<'x', uint32_t>();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    cerr << program;
    exit(1);
  }
  unsigned int max_cycles = program.get<int>("--cycles");
  string image_path = program.get("--image");
  uint32_t mem_size = program.get<uint32_t>("--mem_size");

  println(clog, "Image path  : {}", image_path);
  println(clog, "Max cycles  : {}", max_cycles);
  println(clog, "Memory size : {:x}", mem_size);

  emu = make_unique<NPCemu>(mem_size, image_path);
  simulate(max_cycles);
}