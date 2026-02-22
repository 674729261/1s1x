#pragma once
#include <array>
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
  static constexpr size_t SoundBufferSize = 0x10000;
  static AudioBase_t *curAudioBase;
  using SBF = std::array<uint32_t, SoundBufferSize / sizeof(uint32_t)>;
  std::unique_ptr<SBF> sbuf;
};

inline AudioBase_t Audio;