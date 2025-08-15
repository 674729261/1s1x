#include "Vtop_e7___024root.h"
#include <Vtop_e7.h>
#include <cmath>
#include <cstdint>
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

int main() {
  // 上板测试
  nvboard_bind_all_pins(&dut);
  uint8_t all_high = 0xff;
  nvboard_bind_pin(&all_high, 6, SEG2A, SEG2B, SEG2C, SEG2D, SEG2E, SEG2F);
  nvboard_bind_pin(&all_high, 6, SEG5A, SEG5B, SEG5C, SEG5D, SEG5E, SEG5F);

  nvboard_init();
  design_init();

  while (1) {
    dut.clk = dut.clk ^ 1;
    dut.eval();
    nvboard_update();
  }
}