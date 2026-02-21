#pragma once
#include "my_utils.h"
#include <Setup.h>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <vector>
extern std::vector<uint32_t> mem;

inline size_t init_mem(std::string_view image_path) {
  using std::ifstream;
  using std::ios;

  std::filesystem::path program_path = image_path;
  std::ifstream prog_file(program_path, ios::in | ios::binary);
  if (!prog_file.is_open()) {
    log_and_throw<std::runtime_error>("Failed to open image file : {}",
                                      image_path);
  }
  uint32_t size_prog = 0;
  prog_file.seekg(0, ios::end);
  size_prog = prog_file.tellg();
  prog_file.seekg(0, ios::beg);
  mem.resize((size_prog + 3) / 4);

  prog_file.read(reinterpret_cast<char *>(mem.data()), size_prog);
  prog_file.close();
  spdlog::info("Loaded {} bytes to memory", size_prog);
  return size_prog;
}

extern "C" inline uint32_t mem_read(uint32_t raddr) {
  size_t index = (raddr - config.base_memory) >> 2;
  if (index >= mem.size() || raddr < config.base_memory)
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}]", raddr,
        config.base_memory,
        config.base_memory + config.mem_size * sizeof(uint32_t) - 1);
  return mem[index];
}

extern "C" inline void mem_write(uint32_t waddr, uint32_t wmask,
                                 uint32_t wdata) {
  size_t index = (waddr - config.base_memory) >> 2;
  if (index >= mem.size() || waddr < config.base_memory)
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}]", waddr,
        config.base_memory,
        config.base_memory + config.mem_size * sizeof(uint32_t) - 1);
  uint32_t mask32 = lookup_mask32[wmask];
  mem[index] &= ~mask32;
  mem[index] |= wdata & mask32;
}