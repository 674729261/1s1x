#include "inst.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
uint32_t PC = 0;
uint32_t R[32];
uint32_t M[1 << 24];

void inst_cycle() {
  uint32_t inst = M[PC >> 2];
  uint32_t opcode = get_part(inst, 0, 7);
  switch (opcode) {
  case 0x33:
    // add
    {
      RType rtype = get_RType(inst);
      R[rtype.rd] = R[rtype.rs1] + R[rtype.rs2];
      PC = (PC + 4) % 16;
    }
    break;
  case 0x67:
    // jalr
    {
      IType itype = get_IType(inst);
      R[itype.rd] = (PC + 4);
      PC = sign_extend(itype.imm, 12) + R[itype.rs];
    }
    break;
  case 0x13:
    // addi
    {
      IType itype = get_IType(inst);
      R[itype.rd] = R[itype.rs] + sign_extend(itype.imm, 12);
      PC = (PC + 4) % 16;
    }
    break;
  case 0x37:
    // lui
    {
      UType utype = get_UType(inst);
      R[utype.rd] = utype.imm << 12;
      PC = (PC + 4) % 16;
    }
    break;
  case 0x3:
    // load
    {
      IType itype = get_IType(inst);
      if (itype.funct == 2) // lw
        R[itype.rd] = M[(R[itype.rs] + sign_extend(itype.imm, 12)) >> 2];
      else if (itype.funct == 4) { // lbu
        int pos = R[itype.rs] + sign_extend(itype.imm, 12);
        int bit = pos & 0x3;
        int word = pos & ~0x3;
        R[itype.rd] = (M[word >> 2] >> (bit * 8)) & 0xFF;
      } else
        printf("Illegal load instruction %08x at PC=%08x\n", inst, PC);
      PC = (PC + 4) % 16;
    }
    break;
  case 0x23:
    // save
    {
      SType stype = get_SType(inst);
      if (stype.funct == 2) // sw
        M[(R[stype.rs1] + sign_extend(stype.imm, 12)) >> 2] = R[stype.rs2];
      else if (stype.funct == 4) { // sb
        int pos = R[stype.rs1] + sign_extend(stype.imm, 12);
        int bit = pos & 0x3;
        int word = pos & ~0x3;
        M[word] &= ((~0) ^ (0xFFu << (bit * 8)));
        uint32_t byte = R[stype.rs2] & 0xFF;
        M[word] |= (byte << (bit * 8));
      } else
        printf("Illegal save instruction %08x at PC=%08x\n", inst, PC);
    }
    break;
  default:
    printf("Illegal instruction %08x at PC=%08x\n", inst, PC);
    PC = (PC + 4) % 16;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s <program> <cycles>\n", argv[0]);
    return 1;
  }
  FILE *fp = fopen(argv[1], "rb");
  if (!fp) {
    perror("Failed to open program file");
    return 1;
  }
  int cycles = atoi(argv[2]);
  if (cycles <= 0) {
    printf("Invalid number of cycles: %d\n", cycles);
    fclose(fp);
    return 1;
  }
  for (int i = 0; i < cycles; i++) {
    inst_cycle();
  }
}
