#pragma once
#include "RISCV32.h"
#include <cstdint>
#include <stdexcept>

class NEMUemu : public RISCV32 {
public:
  NEMUemu(size_t MemSize, std::string_view programe);

  addr_t getPC() override final;

  void reset() override final;
  void step(bool display = false, bool record_inst = false,
            bool ftracer = false) override final;
  int instrCount() override final;
  void pause(bool is_paused) override final {}
  void writeMemory(int waddr, int wdata, char wmask) override final;
  uint32_t readMemory(int raddr) override final;
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;

  ~NEMUemu();

private:
  const addr_t PC_Init = 0x80000000u;
  const addr_t memOffset = 0x80000000u;

  int inst_count;
  bool need_sync;

  void *loaded_lib;
  using difftest_memcpy_t = void (*)(uint32_t addr, void *buf, size_t n,
                                     bool direction);
  using difftest_regcpy_t = void (*)(void *dut, bool direction);
  using difftest_exec_t = void (*)(uint64_t n);
  using difftest_init_t = void (*)(int port);

  difftest_memcpy_t difftest_memcpy;
  difftest_regcpy_t difftest_regcpy;
  difftest_exec_t difftest_exec;
  difftest_init_t difftest_init;
};
