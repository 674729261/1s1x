#pragma once
#include "RISCV32.h"
#include <stdexcept>

class Nemu : public RISCV32 {
public:
  Nemu(size_t MemSize, std::string_view programe);

  addr_t getPC() override final;

  void reset() override final;
  void step(bool display = false) override final;
  int instrCount() override final;

  void writeMemory(int waddr, int wdata, char wmask) override final {
    throw std::logic_error("Can not write NEMU memory");
  }
  uint32_t readMemory(int raddr) override final {
    throw std::logic_error("Can not read NEMU memory");
  }
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;

private:
  const addr_t PC_Init = 0x80000000u;
  const addr_t memOffset = 0x80000000u;

  void *loaded_lib;
};
