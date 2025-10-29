#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
struct VideoBase_t {
  uint32_t screen_size_info;
  std::atomic<uint32_t> sync;

  std::unique_ptr<uint8_t[]> vmem1, vmem2;
  std::atomic<uint8_t *> front_ptr;
  uint8_t *back_ptr;
};