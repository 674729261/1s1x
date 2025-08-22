#include <VCPU.h>
#include <cstdint>

uint32_t M[1 << 24] = {0x01400513, 0x010000e7, 0x00c000e7,
                       0x00c00067, 0x00a50513, 0x00008067};

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

int main() {
  design_init();
  const int max_cycle = 16;
  for (int cur_cycle = 0; cur_cycle < max_cycle; cur_cycle++) {
    uint32_t pc = dut.io_pc;
    dut.io_instr = M[pc];
    dut.clock = 0;
    dut.eval();
    dut.clock = 1;
    dut.eval();
  }
}