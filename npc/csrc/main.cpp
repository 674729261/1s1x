#include <VCPU.h>
#include <VCPU___024root.h>
#include <cstdint>
#include <cstdio>

uint32_t M[1 << 24] = {0x01400513, 0x010000e7, 0x00c000e7, 0x01800067,
                       0x00a50513, 0x00008067, 0x555550B7, 0x55500193,
                       0x001181B3, 0x08302023, 0x06300F23, 0x08002203,
                       0x08300203, 0x02C00067};

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

extern "C" int pmem_read(int raddr) { return M[(uint32_t)raddr >> 2]; }
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  for (int i = 0; i < 4; i++) {
    if ((wmask >> i) & 0x1) {
      uint32_t mask32 =
          (uint32_t)((1ull << (8ull * (i + 1))) - (1ull << (8ull * i)));
      uint32_t addr = (uint32_t)waddr >> 2;
      M[addr] &= ~mask32;
      M[addr] |= wdata & mask32;
    }
  }
}

int main(void) {
  design_init();
  const int max_cycle = 16;
  for (int cur_cycle = 0; cur_cycle < max_cycle; cur_cycle++) {
    uint32_t pc = dut.io_pc;
    dut.io_instr = M[pc / 4];
    dut.clock = 0;
    printf("PC = %08x, cycle = %d, R[4] = %08x, M[32] = %08x, M[31] = %08x\n",
           pc, cur_cycle, dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_3_r,
           M[32], M[31]);
    dut.eval();
    dut.clock = 1;
    dut.eval();
  }
}