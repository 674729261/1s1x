#include "Vtop_a.h"
#include "verilated.h"
#include <random>
int main(int argc, char **argv) {
  std::mt19937 rng(0x12345678);
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vtop_a *top = new Vtop_a{contextp};
  while (!contextp->gotFinish()) {
    int a = rng() & 0x1;
    int b = rng() & 0x1;
    top->a = a;
    top->b = b;
    top->eval();
    printf("a = %d, b = %d, f = %d\n", a, b, top->f);
    assert(top->f == (a ^ b));
  }
  delete top;
  delete contextp;
  return 0;
}
