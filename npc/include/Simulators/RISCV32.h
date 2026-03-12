#pragma once
#include "../Tracer/Tracer.h"
#include "Device/Device.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
class RISCV32 {
public:
  RISCV32() : EMUstate(Interrupt::NONE), devices(nullptr) {}

  using addr_t = uint32_t;
  struct CPU_State {
    std::array<uint32_t, 32> gpr;
    addr_t pc;
  };

  enum class Interrupt {
    NONE,   // Program is still alive
    EBREAK, // Hit trap
  };

  static constexpr std::array<std::string, 33> gpr_names = {
      "$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
      "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
      "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6", "pc"};

  static int getGPRIDfromName(std::string_view name) {
    std::string_view remove_dollar(name.begin() + 1, name.end());
    for (int i = 0; i < 32; i++) {
      if (remove_dollar == gpr_names[i])
        return i;
    }
    if (name == "$0")
      return 0;
    if (name == "$pc")
      return 32;
    return -1;
  }

  CPU_State getCPUState() {
    syncCPUState();
    return cpu;
  }

  virtual addr_t getPC() = 0;
  // virtual void writeMemory(int waddr, int wdata, char wmask) = 0;
  // virtual uint32_t readMemory(int raddr) = 0;
  virtual void reset() = 0;
  virtual void step() = 0;
  // Interrupt simulate(unsigned long steps) {
  //   while (steps-- && EMUstate == Interrupt::NONE)
  //     step();
  //   return EMUstate;
  // }

  /**
   * @brief Get the number of instructions simulated since last reset.
   *
   * @return unsigned long long the number of instructions
   */
  virtual unsigned long long instrCount() = 0;

  /**
   * @brief Refresh cached register state. Should be called before calling
   * getGPR.
   *
   */
  virtual void syncCPUState() = 0;
  Interrupt getEMUState() { return EMUstate; }

  /**
   * @brief Get the value of register x_idx.
   *
   * @param idx the index of the register to get.
   * @return uint32_t the value of the register
   */
  virtual uint32_t getGPR(int idx) = 0;
  virtual ~RISCV32() = default;

  void tie_devices(std::shared_ptr<Devices> d) { devices = d; }

protected:
  CPU_State cpu;
  Interrupt EMUstate;

  std::shared_ptr<Devices> devices;
};