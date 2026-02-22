#pragma once
#include <chrono>
#include <cstdint>
struct RTC_t {
  uint32_t RTC_reg[2];
  std::chrono::steady_clock::time_point last_time;
};

inline RTC_t RTC;