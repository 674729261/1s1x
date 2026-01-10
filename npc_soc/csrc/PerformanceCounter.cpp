#include <PerformanceCounter.h>
#include <algorithm>
#include <array>
long long ifu_event;
long long lsu_read_event;
long long exu_event;
long long idu_event;
long long wbu_event;
long long inst_count;
long long clock_count;
long long sum_ifu_fetch_delay;
long long min_ifu_fetch_delay;
long long max_ifu_fetch_delay;
long long sum_lsu_fetch_delay;
long long min_lsu_fetch_delay;
long long max_lsu_fetch_delay;
static long long last_ifu_time;
static long long last_lsu_time;
extern std::unique_ptr<Dut> dut;

std::array<InstTypeItem, 13> inst_type_event = {
    {{0, "Arithmetic immediate"},
     {0, "Arithmetic register"},
     {0, "Store"},
     {0, "Load"},
     {0, "Branch"},
     {0, "Jump and link"},
     {0, "Jump and link register"},
     {0, "Load upper immediate"},
     {0, "Add upper immediate PC"},
     {0, "Environment break"},
     {0, "Environment call"},
     {0, "Machine mode returen"},
     {0, "Control state register operation"}}};

enum {
  TYPE_arithmetic_imm,
  TYPE_arithmetic_reg,
  TYPE_store,
  TYPE_load,
  TYPE_branch,
  TYPE_jal,
  TYPE_jalr,
  TYPE_lui,
  TYPE_auipc,
  TYPE_ebreak,
  TYPE_ecall,
  TYPE_mret,
  TYPE_csrop
};
extern "C" void notify_ifu_r_event() {
  ifu_event++;
  long long delay = dut->sim_time - last_ifu_time;
  sum_ifu_fetch_delay += delay;
  max_ifu_fetch_delay = std::max(max_ifu_fetch_delay, delay);
  min_ifu_fetch_delay = std::min(min_ifu_fetch_delay, delay);
}
extern "C" void notify_lsu_r_event() {
  lsu_read_event++;
  long long delay = dut->sim_time - last_lsu_time;
  sum_lsu_fetch_delay += delay;
  max_lsu_fetch_delay = std::max(max_lsu_fetch_delay, delay);
  min_lsu_fetch_delay = std::min(min_lsu_fetch_delay, delay);
}
extern "C" void notify_ifu_ar_event() { last_ifu_time = dut->sim_time; }
extern "C" void notify_lsu_ar_event() { last_lsu_time = dut->sim_time; }
extern "C" void notify_exu_event() { exu_event++; }
extern "C" void notify_idu_event() { idu_event++; }
extern "C" void notify_wbu_event() { wbu_event++; }
extern "C" void notify_inst_type_is_arithmetic_imm() {
  inst_type_event[TYPE_arithmetic_imm].count++;
}
extern "C" void notify_inst_type_is_arithmetic_reg() {
  inst_type_event[TYPE_arithmetic_reg].count++;
}
extern "C" void notify_inst_type_is_store() {
  inst_type_event[TYPE_store].count++;
}
extern "C" void notify_inst_type_is_load() {
  inst_type_event[TYPE_load].count++;
}
extern "C" void notify_inst_type_is_branch() {
  inst_type_event[TYPE_branch].count++;
}
extern "C" void notify_inst_type_is_jal() { inst_type_event[TYPE_jal].count++; }
extern "C" void notify_inst_type_is_jalr() {
  inst_type_event[TYPE_jalr].count++;
}
extern "C" void notify_inst_type_is_lui() { inst_type_event[TYPE_lui].count++; }
extern "C" void notify_inst_type_is_auipc() {
  inst_type_event[TYPE_auipc].count++;
}
extern "C" void notify_inst_type_is_ebreak() {
  inst_type_event[TYPE_ebreak].count++;
}
extern "C" void notify_inst_type_is_ecall() {
  inst_type_event[TYPE_ecall].count++;
}
extern "C" void notify_inst_type_is_mret() {
  inst_type_event[TYPE_mret].count++;
}
extern "C" void notify_inst_type_is_csrop() {
  inst_type_event[TYPE_csrop].count++;
}

void clear_performance_count() {
  ifu_event = 0;
  lsu_read_event = 0;
  exu_event = 0;
  idu_event = 0;
  wbu_event = 0;
  inst_count = 0;
  clock_count = 0;

  sum_ifu_fetch_delay = 0;
  min_ifu_fetch_delay = 1145141919810;
  max_ifu_fetch_delay = 0;
  sum_lsu_fetch_delay = 0;
  min_lsu_fetch_delay = 1145141919810;
  max_lsu_fetch_delay = 0;

  for (InstTypeItem &item : inst_type_event)
    item.count = 0;
}

void display_performance(auto start_time, auto end_time) {
  auto elapsed_ms =
      std::chrono::floor<std::chrono::milliseconds>(end_time - start_time);

  spdlog::info("Total simulated instructions : {}", inst_count);
  spdlog::info("Total ifu events : {}", ifu_event);
  spdlog::info("Total lsu read events : {}", lsu_read_event);
  spdlog::info("Total exu events : {}", exu_event);
  spdlog::info("Total idu events : {}", idu_event);
  spdlog::info("Total wbu events : {}", wbu_event);
  if (ifu_event > 0)
    spdlog::info("Average/Min/Max IFU delay : {:.2f}/{}/{}",
                 static_cast<double>(sum_ifu_fetch_delay) / ifu_event,
                 min_ifu_fetch_delay, max_ifu_fetch_delay);
  if (lsu_read_event > 0)
    spdlog::info("Average/Min/Max LSU delay : {:.2f}/{}/{}",
                 static_cast<double>(sum_lsu_fetch_delay) / lsu_read_event,
                 min_lsu_fetch_delay, max_lsu_fetch_delay);

  spdlog::info("------------Instruction Type Statistics------------");
  long long sum_recorded_inst = 0;
  for (InstTypeItem &item : inst_type_event) {
    spdlog::info("{:35} |      {}", item.name, item.count);
    sum_recorded_inst += item.count;
  }
  spdlog::info("---------------------------------------------------");
  spdlog::info("{:35} |      {}", "Total", sum_recorded_inst);
  spdlog::info("---------------------------------------------------");

  spdlog::info("Total simulated clock periods : {}", clock_count);
  spdlog::info("Clocks per instruction : {:.3f}",
               static_cast<double>(clock_count) / inst_count);
  spdlog::info("Total simulation time : {:%Hh %Mm %Ss}", elapsed_ms);
  spdlog::info("Simulation speed : {:.2f} clocks/s , {:.2f} insts/s",
               1000 * static_cast<double>(clock_count) / elapsed_ms.count(),
               1000 * static_cast<double>(inst_count) / elapsed_ms.count());
}