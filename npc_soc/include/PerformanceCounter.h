#pragma once
#include "my_utils.h"
#include <algorithm>
#include <chrono>
#include <ranges>
#include <spdlog/spdlog.h>

struct InstTypeItem {
  long long count;
  const std::string name;
};

extern long long ifu_event;
extern long long lsu_event;
extern long long exu_event;
extern long long idu_event;
extern long long wbu_event;
extern long long inst_count;
extern long long clock_count;
extern std::array<InstTypeItem, 13> inst_type_event;

inline void clear_performance_count() {
  ifu_event = 0;
  lsu_event = 0;
  exu_event = 0;
  idu_event = 0;
  wbu_event = 0;
  inst_count = 0;
  clock_count = 0;

  for (InstTypeItem &item : inst_type_event)
    item.count = 0;
}

inline void display_performance(auto start_time, auto end_time) {
  auto elapsed_ms =
      std::chrono::floor<std::chrono::milliseconds>(end_time - start_time);

  spdlog::info("Total simulated instructions : {}", inst_count);
  spdlog::info("Total ifu events : {}", ifu_event);
  spdlog::info("Total lsu events : {}", lsu_event);
  spdlog::info("Total exu events : {}", exu_event);
  spdlog::info("Total idu events : {}", idu_event);
  spdlog::info("Total wbu events : {}", wbu_event);

  spdlog::info("------------Instruction Type Statistics------------");
  for (InstTypeItem &item : inst_type_event) {
    spdlog::info("{:20} : {}", item.name, item.count);
  }
  spdlog::info("---------------------------------------------------");

  spdlog::info("Total simulated clock periods : {}", clock_count);
  spdlog::info("Clocks per instruction : {:.3f}",
               static_cast<double>(clock_count) / inst_count);
  spdlog::info("Total simulation time : {:%Hh %Mm %Ss}", elapsed_ms);
  spdlog::info("Simulation speed : {:.2f} clocks/s , {:.2f} insts/s",
               1000 * static_cast<double>(clock_count) / elapsed_ms.count(),
               1000 * static_cast<double>(inst_count) / elapsed_ms.count());
}