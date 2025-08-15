#include <Vtop_e2.h>
#include <cstdio>
#include <nvboard.h>
#include <random>
static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME *top);

int main() {
  // 自动测试
  std::mt19937 rng(0x12345678);
  int error_count = 0;
  const int test_cases = 256;
  for (int i = 0; i < test_cases; i++) {
    dut.en = 1;
    int vec = rng() & 0xFF;
    dut.penc_input = vec;
    int ans = -1;
    dut.eval();
    for (int j = 0; j < 8; j++) {
      if ((vec >> j) & 1)
        ans = j;
    }
    if (ans == -1 && dut.valid != 0) {
      printf("Error: vec = %02x, valid = %d\n", vec, dut.valid);
      error_count++;
    } else if (ans != -1 && dut.valid == 0) {
      printf("Error: vec = %02x, valid = %d\n", vec, dut.valid);
      error_count++;
    } else if (ans != -1 && dut.penc_output != ans) {
      printf("Error: vec = %02x, output = %d, expected = %d\n", vec,
             dut.penc_output, ans);
      error_count++;
    }
  }
  printf("Test completed with %d errors out of %d cases.\n", error_count,
         test_cases);

  // 上板测试
  nvboard_bind_all_pins(&dut);
  nvboard_init();
  puts("E2 : Encoder and Decoder");
  puts("优先译码器输入 : SW15-8");
  puts("译码器输入 : SW2-1");
  puts("编码器输入 : SW7-4");
  puts("使能 : SW0");
  while (1) {
    nvboard_update();
    dut.eval();
  }
}
