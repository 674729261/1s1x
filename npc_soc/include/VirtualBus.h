#pragma once

#include "my_utils.h"
#include "spdlog/common.h"
#include <MROM.h>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>
struct VirtualBus {
  VirtualBus() : sram(2048) {}
  uint32_t readMemory(uint32_t addr, int sz) {
    if ((addr & (sz - 1)) != 0) {
      log_and_throw<std::logic_error>(
          "Unaligned read in address {:08x}, size = {}", addr, sz);
    }
    if (addr >= mrom_field.from && addr + sz <= mrom_field.to) {
      return mrom_content[(addr & 0x00FFFFFF) >> 2];
    } else if (addr >= sram_field.from && addr + sz <= sram_field.to) {
      return sram[(addr & 0x00FFFFFF) >> 2];
    } else {
      log_and_throw<std::logic_error>(
          "Failed to decode read addr {:08x}, size = {}", addr, sz);
    }
  }

  void writeMemory(uint32_t addr, uint32_t wdata, uint32_t wmask) {
    constexpr std::array<uint32_t, 16> lookup_mask32 = {
        0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF, 0x00FF0000, 0x00FF00FF,
        0x00FFFF00, 0x00FFFFFF, 0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
        0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF};

    if ((addr & 0x3) != 0) {
      log_and_throw<std::logic_error>("Unaligned write to address {:08x}",
                                      addr);
    }
    uint32_t mask32 = lookup_mask32[wmask];
    if (addr >= mrom_field.from && addr <= mrom_field.to) {
      log_and_throw<std::logic_error>(
          "Failed to write to address {:08x} : MROM can not be written", addr);
    } else if (addr >= sram_field.from && addr <= sram_field.to) {
      sram[(addr & 0x00FFFFFF)] &= ~mask32;
      sram[(addr & 0x00FFFFFF)] |= wdata & mask32;
    } else {
      log_and_throw<std::logic_error>("Failed to decode write addr {:08x}",
                                      addr);
    }
  }

  struct Area {
    uint32_t from, to;
  };

  Area mrom_field = {0x20000000, 0x20000fff};

  std::vector<uint32_t> sram;
  Area sram_field = {0x0f000000, 0x0f001fff};
};