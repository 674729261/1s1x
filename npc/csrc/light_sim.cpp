#include "verilated.h"
#include "verilated_vcd_c.h"
#include <Vlight.h>
#include <nvboard.h>
static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME *top);

static void single_cycle() {
  dut.clk = 0;
  dut.eval();
  dut.clk = 1;
  dut.eval();
}

static void reset(int n) {
  dut.rst = 1;
  while (n-- > 0)
    single_cycle();
  dut.rst = 0;
}

int main() {
  Verilated::traceEverOn(true);
  nvboard_bind_all_pins(&dut);
  nvboard_init();
  VerilatedVcdC *vcd = new VerilatedVcdC;
  dut.trace(vcd, 0);
  reset(10);
  vcd->open("wave.vcd");
  for (int i = 0; i < 64; ++i) {
    nvboard_update();
    single_cycle();
    vcd->dump(i);
  }
  vcd->close();
  nvboard_quit();
}
