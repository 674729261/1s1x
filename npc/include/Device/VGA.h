#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
struct VideoBase_t {
  uint32_t screen_size_info;
  std::atomic<uint32_t> sync;

  static constexpr uint32_t ScreenWidth = 400;
  static constexpr uint32_t ScreenHeight = 300;
  static constexpr uint32_t VMemSize =
      ScreenWidth * ScreenHeight * sizeof(uint32_t);
  using VMEM = std::array<uint8_t, VMemSize>;
  std::unique_ptr<VMEM> vmem1, vmem2;
  std::atomic<uint8_t *> front_ptr;
  uint8_t *back_ptr;
};

inline VideoBase_t Video;