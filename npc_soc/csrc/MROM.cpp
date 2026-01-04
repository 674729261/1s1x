#include "spdlog/spdlog.h"
#include <MROM.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <vector>

std::vector<uint32_t> mrom_content;

extern "C" void mrom_read(int32_t addr, int32_t *data) {
  spdlog::info("{} : {}", mrom_content[0x250 / 4], mrom_content[0x254 / 4]);
  *data = mrom_content[(addr & 0x0FFFFFFF) >> 2];
}

int init_mrom(std::string_view image_path) {
  using std::ifstream;
  using std::ios;

  std::filesystem::path program_path = image_path;
  std::ifstream prog_file(program_path, ios::in | ios::binary);
  uint32_t size_prog = 0;
  prog_file.seekg(0, ios::end);
  size_prog = prog_file.tellg();
  prog_file.seekg(0, ios::beg);
  mrom_content.resize((size_prog + 3) / 4);

  prog_file.read(reinterpret_cast<char *>(mrom_content.data()), size_prog);
  prog_file.close();
  for (uint32_t v : mrom_content) {
    std::println("{} ", v);
  }
  return size_prog;
}