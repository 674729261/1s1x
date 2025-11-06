#include <Capstone.h>
#include <ELFParser.h>
#include <Tracer/Tracer.h>
#include <memory>
#include <my_utils.h>
#include <print>
#include <stdexcept>
#include <string_view>

Tracer::Tracer(bool ftracer, std::string_view elf_path, size_t rb_size) {
  if (ftracer) {
    sym_tab = std::make_unique<ProgSymTab>(elf_path);
  }
  if (rb_size > 0) {
    inst_ringbuf = std::make_unique<InstRingBuffer>(rb_size);
  }
  //   if (!capstone.load_libcapstone())
  //     log_and_throw<std::runtime_error>("Failed to load libcapstone");
}

void Tracer::flush_instruction(uint32_t pc, uint32_t inst, uint32_t rs1) {
  if (inst_ringbuf) {
    inst_ringbuf->insert(pc, inst);
  }
  if (sym_tab) {
    record_ftracer(pc, inst, rs1);
  }
  if (display) {
    Capstone::capstone.disassemble(pc, (uint8_t *)&inst, 4, true);
  }
}
void Tracer::show_history_instructions() {
  if (!inst_ringbuf) {
    log_and_throw<std::logic_error>(
        "Tried to show instruction history when inst_ringbuf is unavailable");
  }
  inst_ringbuf->display();
}
void Tracer::record_ftracer(uint32_t pc, uint32_t cur_inst,
                            uint32_t rs1_value) {
  uint32_t opcode = cur_inst & 0x7f;
  uint32_t rd = (cur_inst >> 7) & 0x1f;
  uint32_t dnxt_pc = -1;
  if (opcode == 0x6f) {
    uint32_t imm20 = cur_inst >> 31;
    uint32_t imm10_1 = (cur_inst >> 21) & 0x3ff;
    uint32_t imm11 = (cur_inst >> 20) & 0x1;
    uint32_t imm19_12 = (cur_inst >> 12) & 0xff;
    uint32_t imm =
        (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    imm |= -(imm & 0x80000);
    dnxt_pc = (pc + imm) & ~0x1;
  } else if (opcode == 0x67) {
    // uint32_t rs1 = (cur_inst >> 15) & 0x1f;
    uint32_t imm = cur_inst >> 20;
    imm |= -(imm & 0x800);
    dnxt_pc = (imm + rs1_value) & ~0x1;
  }

  if ((opcode == 0x67 || opcode == 0x6f) && rd == 1) {
    for (int i = 0; i < sym_tab->stack_cnt(); i++)
      std::print(" ");
    int to_symbol = sym_tab->find_symbol_by_addr(dnxt_pc);
    std::println("call {}@{:#010x}", sym_tab->find_symbol_name(to_symbol), pc);
    sym_tab->push_call_stack(to_symbol, pc);
  } else if (cur_inst == 0x00008067) {
    ProgSymTab::Call top = sym_tab->pop_call_stack();
    for (int i = 0; i < sym_tab->stack_cnt(); i++)
      std::print(" ");
    std::println("ret  {}@{:#010x}", sym_tab->find_symbol_name(top.symbol), pc);
  }
}