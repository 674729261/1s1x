#include <Vtop_e6.h>
#include <bitset>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <nvboard.h>
#include <random>
#include <set>
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

uint8_t doit(uint8_t v) {
  int a0 = v & 0x1;
  int a1 = (v >> 1) & 0x1;
  int a2 = (v >> 2) & 0x1;
  int a3 = (v >> 3) & 0x1;

  return (v >> 1) | ((a0 ^ a1 ^ a2 ^ a3) << 7);
}
int main() {
  // 自动测试
  std::mt19937 rng(0x12345678);
  design_init();
  int error_count = 0;
  const int iters = 256;
  for (int i = 0; i < iters; i++) {
    uint8_t v = rng() % 255 + 1;
    dut.load = 1;
    dut.clk = 0;
    dut.initial_value = v;
    dut.eval();
    dut.clk = 1;
    dut.load = 1;
    dut.eval();
    dut.load = 0;
    bool error_flag = false;
    for (int j = 0; j < 256; j++) {
      dut.clk = 0;
      dut.eval();
      dut.clk = 1;
      dut.eval();
      v = doit(v);
      if (v != dut.out) {
        printf("Error: out = %d, expected = %d\n", dut.out, v);
        error_flag = true;
      }
    }
    if (error_flag)
      error_count++;
  }
  printf("Test completed with %d errors out of %d cases.\n", error_count,
         iters);
  // 上板测试
  nvboard_bind_all_pins(&dut);
  uint8_t all_high = 0x7f;
  nvboard_bind_pin(&all_high, 7, SEG2A, SEG2B, SEG2C, SEG2D, SEG2E, SEG2F,
                   SEG2G);
  nvboard_bind_pin(&all_high, 7, SEG3A, SEG3B, SEG3C, SEG3D, SEG3E, SEG3F,
                   SEG3G);

  nvboard_init();
  design_init();

  while (1) {
    dut.eval();
    nvboard_update();
  }
}