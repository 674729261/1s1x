#pragma once
#include "my_utils.h"
#include <Setup.h>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <vector>

inline void write_mask(uint32_t &dst, uint32_t mask32, uint32_t wdata) {
  dst = (dst & ~mask32) | (wdata & mask32);
}

inline void write_mask(std::atomic<uint32_t> &dst, uint32_t mask32,
                       uint32_t wdata) {
  uint32_t t = dst.load(), new_value;
  do {
    new_value = (t & ~mask32) | (wdata & mask32);
  } while (!dst.compare_exchange_weak(t, new_value));
}

inline std::vector<uint32_t> mem;

inline size_t init_mem(std::string_view image_path) {
  using std::ifstream;
  using std::ios;

  mem.resize((config.mem_size + 3) / 4);

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

  prog_file.read(reinterpret_cast<char *>(mem.data()), size_prog);
  prog_file.close();
  spdlog::info("Loaded {} bytes to memory", size_prog);
  return size_prog;
}
