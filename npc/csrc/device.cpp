#include "ports.h"
#include <chrono>
#include <format>
#include <iostream>
#include <optional>

using namespace std::chrono;
using std::cout;
struct {
  uint32_t RTC_reg[2];
  steady_clock::time_point last_time;
} RTC;

static void write_mask(uint32_t &dst, uint32_t mask32, uint32_t wdata) {
  dst &= ~mask32;
  dst |= wdata & mask32;
}

static void update_RTC() {
  auto now_tick = steady_clock().now();
  uint64_t duration = static_cast<uint64_t>(
      duration_cast<microseconds>(now_tick - RTC.last_time).count());
  uint64_t start_time =
      RTC.RTC_reg[0] | (static_cast<uint64_t>(RTC.RTC_reg[1]) << 32);
  uint64_t now_time = start_time + duration;
  RTC.RTC_reg[0] = now_time & 0xFFFFFF;
  RTC.RTC_reg[1] = now_time >> 32;
  RTC.last_time = now_tick;
}

int write_mmio(uint32_t addr, uint32_t mask32, uint32_t wdata) {
  if (addr >= RTC_ADDR && addr < RTC_ADDR_END) {
    uint32_t RTC_id = (addr - RTC_ADDR) >> 2;
    update_RTC();
    write_mask(RTC.RTC_reg[RTC_id], mask32, wdata);
    RTC.last_time = steady_clock().now();
    return 0;
  }
  if (addr == SERIAL_PORT) {
    if (mask32 != 0xFF)
      return -1;
    cout.put(wdata);
    return 0;
  }
  return -1;
}

std::optional<uint32_t> read_mmio(uint32_t raddr) {
  if (raddr >= RTC_ADDR && raddr < RTC_ADDR_END) {
    update_RTC();

    if (raddr == RTC_ADDR)
      return RTC.RTC_reg[0];
    else
      return RTC.RTC_reg[1];
  }
  return std::nullopt;
}