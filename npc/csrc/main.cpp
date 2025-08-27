#include <SDL2/SDL.h>
#include <VCPU.h>
#include <VCPU___024root.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "ports.h"

const size_t Memory_Size = 1 << 24;
uint32_t pc;
uint32_t M[Memory_Size] = {0x01400513, 0x010000e7, 0x00c000e7, 0x01800067,
                           0x00a50513, 0x00008067, 0x555550B7, 0x55500193,
                           0x001181B3, 0x08302023, 0x06300F23, 0x08002203,
                           0x08300203, 0x00100073};

const uint32_t PC_Init = 0x80000000u;

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
int trapped;
extern "C" void trap(int signal) { trapped = signal; }
extern "C" int pmem_read(int raddr) {
  raddr &= ~0x3;
  uint32_t addr = (uint32_t)(raddr - PC_Init) >> 2;
  if (addr < Memory_Size && raddr >= PC_Init)
    return M[addr];
  if (raddr >= DEVICE_BASE) {
    auto ret = read_mmio(raddr);
    if (!ret.has_value()) {
      return 0xdeafbeef;
    }
    return ret.value();
  }

  return 0xdeafbeef;
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  for (int i = 0; i < 4; i++) {
    if ((wmask >> i) & 0x1) {
      uint32_t mask32 =
          (uint32_t)((1ull << (8ull * (i + 1))) - (1ull << (8ull * i)));
      uint32_t addr = (uint32_t)(waddr - PC_Init) >> 2;
      if (addr < Memory_Size && waddr >= PC_Init) {
        M[addr] &= ~mask32;
        M[addr] |= wdata & mask32;
      } else if (waddr >= DEVICE_BASE) {
        if (write_mmio(waddr & ~0x3, mask32, wdata) < 0) {
          printf("waddr : %08x invalid device\n", waddr);
          exit(-1);
        }
      } else {
        printf("waddr : %08x invalid address\n", waddr);
        exit(-1);
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: %s [prog] [a/b] (num of max cycles)\n", argv[0]);
    exit(0);
  }
  FILE *fp = fopen(argv[1], argv[2][0] == 'a' ? "r" : "rb");
  if (!fp) {
    perror("Failed to open program file");
    return 1;
  }
  uint32_t curpos = 0;
  if (argv[2][0] == 'a')
    while (fscanf(fp, "%x", &M[curpos++]) != EOF)
      ;
  else
    while (!feof(fp)) {
      fread(M + curpos, sizeof(uint32_t), 1, fp);
      curpos++;
    }
  printf("Loaded %d words\n", curpos);
  fclose(fp);
  design_init();
  const unsigned int max_cycle = argc >= 4 ? atoi(argv[3]) : UINT32_MAX;
  unsigned int cur_cycle;
  for (cur_cycle = 0; cur_cycle < max_cycle; cur_cycle++) {
    pc = dut.io_pc;
    if (pc < PC_Init) {
      printf("pc : %08x out of range\n", pc);
      exit(-1);
    }
    dut.io_instr = M[(pc - PC_Init) / 4];
    dut.clock = 0;
    dut.eval();
    dut.clock = 1;
    dut.eval();

    if (trapped) {
      printf("EBREAK, a0 = %08x, pc = %08x, cycle = %d\n",
             dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_9_r, pc,
             cur_cycle);
      if (dut.rootp->CPU__DOT__gpr__DOT__register_bank_regs_9_r == 0) {
        puts("HIT GOOD TRAP");
        return 0;
      } else {
        puts("HIT BAD TRAP");
        return -1;
      }
      break;
    }
  }
  if (cur_cycle == max_cycle) {
    printf("Fail to halt, Abort at pc = %08x\n", dut.io_pc);
    return -1;
  }
}