#include <VRNG.h>
#include <cmath>
#include <cstdint>
#include <random>

static TOP_NAME dut;

void design_init() {
  dut.reset = 1;
  for (int i = 0; i < 4; ++i) {

    dut.clock = 0;
    dut.eval();
    dut.clock = 1;
    dut.eval();
  }
  dut.reset = 0;
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
  std::mt19937 rng(0x12345678);
  design_init();
  int error_count = 0;
  const int iters = 256;
  dut.io_en = 1;
  for (int i = 0; i < iters; i++) {
    uint8_t v = rng() % 255 + 1;
    dut.io_load = 1;
    dut.clock = 0;
    dut.io_load_data = v;
    dut.eval();
    dut.clock = 1;
    dut.io_load = 1;
    dut.eval();
    dut.io_load = 0;
    bool error_flag = false;
    for (int j = 0; j < 256; j++) {
      dut.clock = 0;
      dut.eval();
      dut.clock = 1;
      dut.eval();
      v = doit(v);
      if (v != dut.io_out) {
        printf("Error: out = %d, expected = %d\n", dut.io_out, v);
        error_flag = true;
      }
    }
    if (error_flag)
      error_count++;
  }
  printf("Test completed with %d errors out of %d cases.\n", error_count,
         iters);

  design_init();

  puts("随机数发生器");
}