#include <Vtop_e4.h>
#include <chrono>
#include <cstdio>
#include <nvboard.h>
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
  nvboard_init();
  design_init();

  auto last = std::chrono::steady_clock::now();
  while (1) {
    auto now = std::chrono::steady_clock::now();
    if (now - last > std::chrono::nanoseconds(1000)) {
      last = now;
      dut.clk = dut.clk ^ 1;
      dut.eval();
      nvboard_update();
    }
  }
}