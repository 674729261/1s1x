#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
class RISCV32 {
public:
  using addr_t = uint32_t;
  struct CPU_State {
    std::array<uint32_t, 32> gpr;
    addr_t pc;
  };

  enum class Interrupt { NONE, EBREAK };

  RISCV32(size_t MemSize, std::string_view program, addr_t init_pc);
  static constexpr std::array<std::string, 32> gpr_names = {
      "$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
      "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
      "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

  static int getGPRIDfromName(std::string_view name) {
    if (name == "$0")
      return 0;
    std::string_view remove_dollar(name.begin() + 1, name.end());
    for (int i = 0; i < 32; i++) {
      if (remove_dollar == gpr_names[i])
        return i;
    }
    return -1;
  }

  virtual CPU_State getCPUState() = 0;

  virtual addr_t getPC() = 0;
  virtual void writeMemory(int waddr, int wdata, char wmask) = 0;
  virtual uint32_t readMemory(int raddr) = 0;
  virtual void reset() = 0;
  virtual Interrupt step(std::size_t c) = 0;
  virtual int instrCount() = 0;
  virtual void syncCPUState() = 0;
  Interrupt getEMUState() { return EMUstate; }
  uint32_t getGPR(int idx) {
    assert(idx >= 0 && idx < 32);
    return cpu.gpr[idx];
  }
  virtual ~RISCV32() = default;

protected:
  std::vector<uint32_t> M;
  CPU_State cpu;
  Interrupt EMUstate;
};