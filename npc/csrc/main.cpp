#include "Simulators/NPCemu.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstdint>
#include <memory>
#include <string>

using std::make_unique;
using std::unique_ptr;

unique_ptr<NPCemu> emu;

extern "C" void trap(int signal) { emu->trapped = 1; }
extern "C" int pmem_read(int raddr) { return emu->readMemory(raddr); }
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  emu->writeMemory(waddr, wdata, wmask);
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
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }
  unsigned int max_cycles = program.get<int>("--cycles");
  std::string image_path = program.get("--image");
  uint32_t mem_size = program.get<uint32_t>("--mem_size");
  emu = make_unique<NPCemu>(mem_size, image_path);
}