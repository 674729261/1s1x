#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
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

  virtual CPU_State getCPUState() = 0;
  virtual uint32_t getGPR(int idx) = 0;
  virtual addr_t getPC() = 0;

  virtual void reset() = 0;
  virtual Interrupt step(std::size_t c) = 0;
  virtual int instrCount() = 0;

protected:
  std::vector<uint32_t> M;
  CPU_State cpu;
};