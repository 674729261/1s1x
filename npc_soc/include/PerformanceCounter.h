#pragma once
#include <string>

struct InstTypeItem {
  long long count;
  const std::string name;
};

extern long long ifu_event;
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
extern std::array<InstTypeItem, 13> inst_type_event;

void clear_performance_count();
void display_performance(auto start_time, auto end_time);