#include "Vlight.h"
#include "verilated.h"
#include <random>

void single_cycle(Vlight *top) {
  top->clk = 0;
  top->eval();
  top->clk = 1;
  top->eval();
}

void reset(Vlight *top, int n) {
  top->rst = 1;
  while (n-- > 0)
    single_cycle(top);
  top->rst = 0;
}

int main(int argc, char **argv) {
  std::mt19937 rng(0x12345678);
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vlight *top = new Vlight{contextp};
  reset(top, 10);
  while (!contextp->gotFinish()) {
    single_cycle(top);
  }
  delete top;
  delete contextp;
  return 0;
}
