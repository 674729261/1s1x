#include <Simulators/RISCV32.h>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>

RISCV32::RISCV32(size_t MemSize, std::string_view program, addr_t init_pc)
    : EMUstate(Interrupt::NONE) {
  using std::ifstream;
  using std::ios;
  M.resize(MemSize / 4);
  std::filesystem::path program_path = program;
  std::ifstream prog_file(program_path, ios::in | ios::binary);
  if (!prog_file.good()) {
    throw std::runtime_error(
        std::format("Failed to open program file {}", program));
  }
  uint32_t size_prog = 0;
  prog_file.seekg(0, ios::end);
  size_prog = prog_file.tellg();
  prog_file.seekg(0, ios::beg);
  if (size_prog > MemSize * sizeof(uint32_t))
    throw std::logic_error(
        std::format("Program size is bigger than memory size {}", MemSize));

  prog_file.read(reinterpret_cast<char *>(M.data()), size_prog);

  spdlog::info("Loaded {} words", size_prog);
  cpu.pc = init_pc;
  prog_file.close();
}
