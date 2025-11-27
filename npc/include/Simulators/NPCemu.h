#pragma once

#include "RISCV32.h"
#include "verilated.h"
#include <Device/Audio.h>
#include <Device/Device.h>
#include <Device/Keyboard.h>
#include <Device/VGA.h>
#include <Vnpc_top.h>
#include <cstdint>
#include <lockfree/spsc/queue.hpp>
#include <print>

extern "C" void trap(int signal);
extern "C" int pmem_read(int raddr, int clk, int valid);
extern "C" void pmem_write(int waddr, int wdata, char wmask);

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

class ProgSymTab;

class NPCemu : public RISCV32 {
public:
  NPCemu();

  addr_t getPC() override final;

  void reset() override final;
  void step() override final;
  unsigned long long instrCount() override final;

  // void writeMemory(int waddr, int wdata, char wmask) override final;
  // uint32_t readMemory(int raddr) override final;
  uint32_t getGPR(int idx) override final;

  void syncCPUState() override final;

  // friend void trap(int signal);
  // friend int pmem_read(int raddr);
  // friend void pmem_write(int waddr, int wdata, char wmask);

  ~NPCemu();

private:
  VerilatedContext context;
  TOP_NAME dut;
  int trapped;
  unsigned long long inst_count;

private:
  // void record_ftracer(uint32_t cur_inst, std::shared_ptr<ProgSymTab> sy_tab);
};
