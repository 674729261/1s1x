#include "Simulators/RISCV32.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string_view>

RISCV32::RISCV32(size_t MemSize, std::string_view program, addr_t init_pc) {
  using std::ifstream;
  using std::ios;
  M.resize(MemSize);
  std::filesystem::path program_path = program;
  std::ifstream prog_file(program_path, ios::in | ios::binary);
  if (!prog_file.good()) {
    throw std::runtime_error(
        std::format("Failed to open program file {}", program));
  }
  uint32_t curpos = 0;
  while (!prog_file.eof()) {
    if (curpos == MemSize)
      throw std::runtime_error(
          std::format("Program size is bigger than memory size {}", MemSize));
    prog_file.read(reinterpret_cast<char *>(M.data() + curpos),
                   sizeof(uint32_t));
    curpos++;
  }
  cpu.pc = init_pc;
  prog_file.close();
}
