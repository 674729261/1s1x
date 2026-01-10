long long ifu_event;
long long lsu_event;
long long exu_event;
long long idu_event;
long long wbu_event;
long long inst_count;
long long clock_count;

extern "C" void notify_ifu_event() { ifu_event++; }
extern "C" void notify_lsu_event() { lsu_event++; }
extern "C" void notify_exu_event() { exu_event++; }
extern "C" void notify_idu_event() { idu_event++; }
extern "C" void notify_wbu_event() { wbu_event++; }
extern "C" void notify_inst_type_is_arithmetic_imm() {}
extern "C" void notify_inst_type_is_arithmetic_reg() {}
extern "C" void notify_inst_type_is_store() {}
extern "C" void notify_inst_type_is_load() {}
extern "C" void notify_inst_type_is_branch() {}
extern "C" void notify_inst_type_is_jal() {}
extern "C" void notify_inst_type_is_jalr() {}
extern "C" void notify_inst_type_is_lui() {}
extern "C" void notify_inst_type_is_auipc() {}
extern "C" void notify_inst_type_is_ebreak() {}
extern "C" void notify_inst_type_is_ecall() {}
extern "C" void notify_inst_type_is_mret() {}
extern "C" void notify_inst_type_is_csrop() {}
