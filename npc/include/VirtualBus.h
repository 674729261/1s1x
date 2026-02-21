#pragma once

#include <cstdint>
#include <my_utils.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

struct VirtualBus {
  VirtualBus() : ram(2048) {}

  struct ReadResult {
    uint32_t data;
    bool read_nonmemory;
  };
  ReadResult readMemory(uint32_t addr, int sz) {
    if ((addr & (sz - 1)) != 0) {
      log_and_throw<std::logic_error>(
          "Unaligned read in address {:08x}, size = {}", addr, sz);
    }

    auto check_range = [=](Area area) {
      return addr >= area.from && addr + sz - 1 <= area.to;
    };

    if (check_range(ram_field)) {
      return {ram[(addr & 0x00FFFFFF) >> 2], false};
    } else if (check_range(keyboard_field)) {
      return {0xdeadbeef, true};
    } else if (check_range(vga_field)) {
      return {0xdeadbeef, true};
    } else if (check_range(audio_field)) {
      return {0xdeadbeef, true};
    } else if (check_range(keyboard_field)) {
      return {0xdeadbeef, true};
    } else {
      log_and_throw<std::logic_error>(
          "Failed to decode read addr {:08x}, size = {}", addr, sz);
    }
  }

  void writeMemory(uint32_t addr, uint32_t wdata, uint32_t wmask) {
    auto check_range = [=](Area area) {
      return addr >= area.from && addr <= area.to;
    };
    if ((addr & 0x3) != 0) {
      log_and_throw<std::logic_error>("Unaligned write to address {:08x}",
                                      addr);
    }
    uint32_t mask32 = lookup_mask32[wmask];
    if (check_range(ram_field)) {
      ram[(addr & 0x00FFFFFF) >> 2] &= ~mask32;
      ram[(addr & 0x00FFFFFF) >> 2] |= wdata & mask32;
    } else if (check_range(keyboard_field)) {
      log_and_throw<std::logic_error>(
          "Failed to write to address {:} : keyboard can not be written", addr);
    } else if (check_range(vga_field)) {
      // no action
    } else if (check_range(keyboard_field)) {
      // no action
    } else if (check_range(audio_field)) {
      // no action
    } else {
      log_and_throw<std::logic_error>("Failed to decode write addr {:08x}",
                                      addr);
    }
  }

  struct Area {
    uint32_t from, to;
  };

  std::vector<uint32_t> ram;
  const Area ram_field = {0x0f000000, 0x0f001fff};

  const Area uart_field = {0x10000000, 0x10000fff};
  const Area audio_field = {0x10001000, 0x10001fff};
  const Area keyboard_field = {0x10011000, 0x10011007};
  const Area vga_field = {0x21000000, 0x211fffff};
};