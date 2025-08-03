#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
uint8_t PC = 0;
uint8_t R[4];
uint8_t M[16] = {0x8a, 0x90, 0xa1, 0xb0, 0x16, 0x3d, 0xd1, 0x43};

void inst_cycle() {
  uint8_t inst = M[PC];

  uint8_t opcode = inst >> 6;
  uint8_t rd, rs1, rs2, imm, addr;
  switch (opcode) {
  case 0:
    rd = (inst >> 4) & 0x3;
    rs1 = (inst >> 2) & 0x3;
    rs2 = inst & 0x3;
    R[rd] = R[rs1] + R[rs2];
    PC = (PC + 1) % 16;
    break;
  case 1:
    rs2 = inst & 0x3;
    printf("R[%d] = %d\n", rs2, R[rs2]);
    PC = (PC + 1) % 16;
    puts("Program ends");
    exit(0);
  case 2:
    rd = (inst >> 4) & 0x3;
    imm = inst & 0xf;
    R[rd] = imm;
    PC = (PC + 1) % 16;
    break;
  case 3:
    rs2 = inst & 0x3;
    addr = (inst >> 2) & 0xf;
    if (R[0] != R[rs2]) {
      PC = addr;
    } else {
      PC = (PC + 1) % 16;
    }
    break;
  default:
    printf("Illegal instruction %02x at PC=%02x\n", inst, PC);
    PC = (PC + 1) % 16;
  }
}

int main(int argc, char *argv[]) {
  while (1) {
    inst_cycle();
  }
}
