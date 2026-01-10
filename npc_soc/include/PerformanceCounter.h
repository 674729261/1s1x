#pragma once
extern long long ifu_event;
extern long long lsu_event;
extern long long exu_event;
extern long long idu_event;
extern long long inst_count;
extern long long clock_count;

inline void clear_performance_count() {
  ifu_event = 0;
  lsu_event = 0;
  exu_event = 0;
  idu_event = 0;
  inst_count = 0;
  clock_count = 0;
}