#pragma once
#include "DUT.h"
#include <chrono>
#include <spdlog/spdlog.h>

struct InstTypeItem {
  long long count;
  const std::string name;
};

extern long long ifu_event;
extern long long icache_hit_event;
extern long long lsu_read_event;
extern long long exu_event;
extern long long idu_event;
extern long long wbu_event;
extern long long inst_count;
extern long long clock_count;
extern long long sum_ifu_fetch_delay;
extern long long min_ifu_fetch_delay;
extern long long max_ifu_fetch_delay;
extern long long sum_lsu_fetch_delay;
extern long long min_lsu_fetch_delay;
extern long long max_lsu_fetch_delay;

extern long long cycles_not_on_flash;
extern long long insts_not_on_flash;

extern long long simulation_time;
extern long long simulation_clocks;
extern long long simulation_instructions;

extern std::array<InstTypeItem, 13> inst_type_event;

extern std::unique_ptr<Dut> dut;

inline void clear_performance_count() {
  ifu_event = 0;
  icache_hit_event = 0;
  lsu_read_event = 0;
  exu_event = 0;
  idu_event = 0;
  wbu_event = 0;
  inst_count = 0;
  clock_count = 0;
  cycles_not_on_flash = 0;
  insts_not_on_flash = 0;

  sum_ifu_fetch_delay = 0;
  min_ifu_fetch_delay = 1145141919810;
  max_ifu_fetch_delay = 0;
  sum_lsu_fetch_delay = 0;
  min_lsu_fetch_delay = 1145141919810;
  max_lsu_fetch_delay = 0;

  for (InstTypeItem &item : inst_type_event)
    item.count = 0;
}
