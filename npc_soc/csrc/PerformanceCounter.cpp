
#include <iostream>
#include <ostream>
long long ifu_event;
long long lsu_event;
long long exu_event;
long long idu_event;
long long inst_count;
long long clock_count;

extern "C" void notify_ifu_event() {
  std::println(std::cout, "!!");
  ifu_event++;
}
extern "C" void notify_lsu_event() { lsu_event++; }
extern "C" void notify_exu_event() { exu_event++; }
extern "C" void notify_idu_event() { idu_event++; }