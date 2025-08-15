#include <VsCPU.h>
#include <VsCPU___024root.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <nvboard.h>
#include <random>
static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME *top);

void design_init() {
  dut.rst = 1;
  for (int i = 0; i < 4; ++i) {

    dut.clk = 0;
    dut.eval();
    dut.clk = 1;
    dut.eval();
  }
  dut.rst = 0;
  dut.eval();
}
uint8_t instr[16] = {0x8a, 0x90, 0xa1, 0xb0, 0x16, 0x3d, 0xd1, 0x43};
int main() {
  // 上板测试
  nvboard_bind_all_pins(&dut);
  uint8_t all_high = 0xff;
  nvboard_bind_pin(&all_high, 7, SEG2A, SEG2B, SEG2C, SEG2D, SEG2E, SEG2F,
                   SEG2G);

  nvboard_init();
  design_init();

  auto last = std::chrono::steady_clock::now();
  while (1) {
    auto now = std::chrono::steady_clock::now();
    if (now - last > std::chrono::milliseconds(200)) {
      last = now;
      printf("PC = %02X ", dut.rootp->sCPU__DOT__pc);
      printf("GPR = %4d %4d %4d %4d\n",
             dut.rootp->sCPU__DOT__sCPU_GPR__DOT__GPRdata[0],
             dut.rootp->sCPU__DOT__sCPU_GPR__DOT__GPRdata[1],
             dut.rootp->sCPU__DOT__sCPU_GPR__DOT__GPRdata[2],
             dut.rootp->sCPU__DOT__sCPU_GPR__DOT__GPRdata[3]);

      dut.clk = 0;
      dut.eval();
      dut.clk = 1;
      dut.eval();
      dut.ins_data = instr[dut.ins_addr];
      nvboard_update();
    }
  }
}