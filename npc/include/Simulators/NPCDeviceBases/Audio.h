#pragma once
#include <cstdint>
#include <memory>
struct AudioBase_t {
  struct {
    uint32_t reg_freq;
    uint32_t reg_channels;
    uint32_t reg_samples;
    uint32_t reg_sbuf_size;
    uint32_t reg_init;
    uint32_t reg_count;
  } reg_ctl;

  static constexpr int n_regs = 6;

  std::unique_ptr<uint8_t[]> sbuf;
};