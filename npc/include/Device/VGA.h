#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
struct VideoBase_t {

  std::atomic<uint32_t> sync;

  static constexpr uint32_t ScreenWidth = 400;
  static constexpr uint32_t ScreenHeight = 300;
  static constexpr uint32_t screen_size_info =
      (ScreenWidth << 16) | ScreenHeight;
  static constexpr uint32_t VMemSize =
      ScreenWidth * ScreenHeight * sizeof(uint32_t);
  using VMEM = std::array<uint32_t, VMemSize / sizeof(uint32_t)>;
  std::unique_ptr<VMEM> vmem1, vmem2;
  std::atomic<uint32_t *> front_ptr;
  uint32_t *back_ptr;
};

inline VideoBase_t Video;