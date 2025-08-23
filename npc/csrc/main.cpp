#include <VCPU.h>
#include <VCPU___024root.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

const size_t Memory_Size = 1 << 24;

uint32_t M[Memory_Size] = {0x01400513, 0x010000e7, 0x00c000e7, 0x01800067,
                           0x00a50513, 0x00008067, 0x555550B7, 0x55500193,
                           0x001181B3, 0x08302023, 0x06300F23, 0x08002203,
                           0x08300203, 0x00100073};

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
extern "C" int pmem_read(int raddr) {
  uint32_t pos = (uint32_t)(raddr - 0x80000000u) >> 2;
  if (pos > Memory_Size)
    return 0xdeadbeef;
  return M[pos];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  if (waddr < 0x80000000u) {
    printf("waddr : %08x out of range\n", waddr);
    exit(-1);
  }
  for (int i = 0; i < 4; i++) {
    if ((wmask >> i) & 0x1) {
      uint32_t mask32 =
          (uint32_t)((1ull << (8ull * (i + 1))) - (1ull << (8ull * i)));
      uint32_t addr = (uint32_t)(waddr - 0x80000000u) >> 2;
      M[addr] &= ~mask32;
      M[addr] |= wdata & mask32;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s [prog] (num of max cycles)\n", argv[0]);
    exit(0);
  }
  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    perror("Failed to open program file");
    return 1;
  }
  uint32_t curpos = 0;
  while (fscanf(fp, "%x", &M[curpos++]) != EOF)
    ;
  printf("Loaded %d words\n", curpos);
  fclose(fp);
  design_init();
  const unsigned int max_cycle = argc >= 2 ? atoi(argv[2]) : UINT32_MAX;
  unsigned int cur_cycle;
  for (cur_cycle = 0; cur_cycle < max_cycle; cur_cycle++) {
    uint32_t pc = dut.io_pc;
    if (pc < 0x80000000u) {
      printf("pc : %08x out of range\n", pc);
      exit(-1);
    }
    dut.io_instr = M[(pc - 0x80000000u) / 4];
    dut.clock = 0;
    dut.eval();
    dut.clock = 1;
    dut.eval();

    if (dut.io_ebreak) {
      printf("EBREAK, a0 = %08x, pc = %08x, cycle = %d\n",
             dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_9_r, pc,
             cur_cycle);
      if (dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_9_r == 0)
        puts("HIT GOOD TRAP");
      else
        puts("HIT BAD TRAP");
      break;
    }
  }
  if (cur_cycle == max_cycle) {
    printf("Fail to halt, Abort at pc = %08x\n", dut.io_pc);
  }
}