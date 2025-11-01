#pragma once

#include "../ELFParser.h"
#include "../RingBuffer.hpp"
#include <cstdint>
#include <memory>
class Tracer {
public:
  Tracer(bool ftracer, std::string_view elf_path, size_t rb_size);

  void flush_instruction(uint32_t pc, uint32_t inst, uint32_t rs1);
  void show_history_instructions();

  void set_display(bool display) { this->display = display; }

private:
  void record_ftracer(uint32_t pc, uint32_t cur_inst, uint32_t rs1);

private:
  std::unique_ptr<ProgSymTab> sym_tab;
  std::unique_ptr<InstRingBuffer> inst_ringbuf;
  //   Capstone capstone;

  bool display;
};