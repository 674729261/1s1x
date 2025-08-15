#include "Vsw.h"
#include "verilated.h"

#include "verilated_vcd_c.h"
#include <random>

int main(int argc, char **argv) {
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  VerilatedVcdC *tfp = new VerilatedVcdC;
  contextp->traceEverOn(true);
  Vsw *top = new Vsw{contextp};
  top->trace(tfp, 0);
  std::mt19937 mt(1234);
  tfp->open("wave.vcd");
  while (!contextp->gotFinish() && contextp->time() < 64) {
    top->a = mt() % 2;
    top->b = mt() % 2;
    top->eval();
    tfp->dump(contextp->time());
    contextp->timeInc(1);
  }
  delete top;
  delete contextp;
  tfp->close();
  return 0;
}
