#include "Capstone.h"
#include <cstddef>
#include <cstdint>
#include <print>
#include <stdexcept>
#include <vector>
class InstRingBuffer {
public:
  struct Item {
    uint32_t pc;
    uint32_t instr;
  };

  void init(size_t sz) {
    instr_buffer.resize(sz);
    pos_begin = 0;
    pos_end = sz - 1;
    cnt = 0;
  }
  void insert(uint32_t pc, uint32_t instr) {
    instr_buffer[pos_begin] = {.pc = pc, .instr = instr};
    pos_end++;
    if (cnt < instr_buffer.size())
      cnt++;
    else {
      pos_begin++;
      if (pos_begin == instr_buffer.size())
        pos_begin = 0;
    }
    if (pos_end == instr_buffer.size())
      pos_end = 0;
  }

  Item last(uint32_t instr) {
    if (cnt == 0)
      throw std::logic_error(
          "Can not fetch last element of an empty RingBuffer");
    return instr_buffer[pos_end];
  }

  Item first(uint32_t instr) {
    if (cnt == 0)
      throw std::logic_error(
          "Can not fetch first element of an empty RingBuffer");
    return instr_buffer[pos_begin];
  }

  void display() {
    if (cnt == 0)
      std::println("No instruction recorded");
    else {
      std::println("Recent {} instructions", cnt);
      int pos = pos_begin;
      for (int i = 0; i < cnt; i++) {
        auto show = Capstone::capstone.disassemble(
            instr_buffer[pos].pc, (uint8_t *)&instr_buffer[pos].instr, 4,
            false);
        if (i != cnt - 1)
          println("{:3}     {}", i, show);
        else
          println("{:3}---->{}", i, show);
        pos++;
        if (pos == instr_buffer.size())
          pos = 0;
      }
    }
  }

  static InstRingBuffer instRingBuffer;

private:
  InstRingBuffer() : instr_buffer(), pos_begin(), pos_end(), cnt() {}
  std::vector<Item> instr_buffer;
  int pos_begin, pos_end, cnt;
};

inline InstRingBuffer InstRingBuffer::instRingBuffer{};