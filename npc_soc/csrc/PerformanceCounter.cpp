#include <PerformanceCounter.h>
#include <array>
long long ifu_event;
long long lsu_event;
long long exu_event;
long long idu_event;
long long wbu_event;
long long inst_count;
long long clock_count;
long long sum_ifu_fetch_delay;
long long sum_lsu_fetch_delay;
static long long last_ifu_time;
static long long last_lsu_time;

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
  sum_ifu_fetch_delay += dut->sim_time - last_ifu_time;
}
extern "C" void notify_lsu_r_event() {
  lsu_event++;
  sum_lsu_fetch_delay += dut->sim_time - last_lsu_time;
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
