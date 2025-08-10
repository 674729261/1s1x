#include "inst.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE (1 << 24)
int ebreak = 0;
uint32_t PC = 0;
uint32_t R[32];
uint32_t M[MEMORY_SIZE];

void inst_cycle() {
  uint32_t inst = M[PC >> 2];

  if (inst == 0x100073) // ebreak
  {
    ebreak = 1;
    return;
  }

  uint32_t opcode = get_part(inst, 0, 7);

  switch (opcode) {
  case 0x33:
    // add
    {
      RType rtype = get_RType(inst);
      R[rtype.rd] = R[rtype.rs1] + R[rtype.rs2];
      PC = (PC + 4) % MEMORY_SIZE;
    }
    break;
  case 0x67:
    // jalr
    {
      IType itype = get_IType(inst);
      uint32_t new_PC = (sign_extend(itype.imm, 12) + R[itype.rs]) & ~1u;
      R[itype.rd] = (PC + 4) % MEMORY_SIZE;
      PC = new_PC;
    }
    break;
  case 0x13:
    // addi
    {
      IType itype = get_IType(inst);
      R[itype.rd] = R[itype.rs] + sign_extend(itype.imm, 12);
      PC = (PC + 4) % MEMORY_SIZE;
    }
    break;
  case 0x37:
    // lui
    {
      UType utype = get_UType(inst);
      R[utype.rd] = (utype.imm << 12);
      PC = (PC + 4) % MEMORY_SIZE;
    }
    break;
  case 0x3:
    // load
    {
      IType itype = get_IType(inst);
      if (itype.funct == 2) // lw
      {
        if (((R[itype.rs] + sign_extend(itype.imm, 12)) & 0x3) != 0)
          printf("Unaligned memory address in instr %08x in %08x\n", M[PC >> 2],
                 PC);
        R[itype.rd] = M[(R[itype.rs] + sign_extend(itype.imm, 12)) >> 2];
      } else if (itype.funct == 4) { // lbu
        uint32_t pos = R[itype.rs] + sign_extend(itype.imm, 12);
        uint32_t bit = pos & 0x3;
        R[itype.rd] = (M[pos >> 2] >> (bit * 8)) & 0xFF;
      } else
        printf("Illegal load instruction %08x at PC=%08x\n", inst, PC);
      PC = (PC + 4) % MEMORY_SIZE;
    }
    break;
  case 0x23:
    // save
    {
      SType stype = get_SType(inst);
      if (stype.funct == 2) // sw
      {
        if (((R[stype.rs1] + sign_extend(stype.imm, 12)) & 0x3) != 0)
          printf("Unaligned memory address in instr %08x in %08x\n", M[PC >> 2],
                 PC);
        M[(R[stype.rs1] + sign_extend(stype.imm, 12)) >> 2] = R[stype.rs2];
      } else if (stype.funct == 0) { // sb
        uint32_t pos = R[stype.rs1] + sign_extend(stype.imm, 12);
        uint32_t bit = pos & 0x3;
        M[pos >> 2] &= ((~0) ^ (0xFFu << (bit * 8)));
        uint32_t byte = R[stype.rs2] & 0xFF;
        M[pos >> 2] |= (byte << (bit * 8));
      } else
        printf("Illegal save instruction %08x at PC=%08x\n", inst, PC);
      PC = (PC + 4) % MEMORY_SIZE;
    }
    break;
  default:
    printf("Illegal instruction %08x at PC=%08x\n", inst, PC);
    PC = (PC + 4) % MEMORY_SIZE;
  }

  R[0] = 0;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage: %s <program> <cycles> <ebreak_pos(decimal)>\n", argv[0]);
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

  int ebreak_pos = atoi(argv[3]);

  int curpos = 0;
  while (fscanf(fp, "%x", &M[curpos++]) != EOF)
    ;
  M[ebreak_pos >> 2] = 0x100073;
  printf("Load %d word(s) from program file %s\n", curpos, argv[1]);
  fclose(fp);
  puts("Begin simulation");
  int emu_cycles;
  for (emu_cycles = 0; emu_cycles < cycles && !ebreak; emu_cycles++) {
    inst_cycle();
  }

  if (ebreak)
    puts("Program terminated by ebreak");
  else
    puts("Clock cycles run out");
  printf("PC = %08x after %d cycle(s)\n", PC, emu_cycles);
  puts("Registers:");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++)
      printf("%08x ", R[i * 8 + j]);
    putchar('\n');
  }

  if (ebreak) {
    if (R[10] == 0)
      puts("Register a0 = 0, HIT GOOD TRAP");
    else
      puts("Register a0 != 0, HIT BAD TRAP");
  }
}