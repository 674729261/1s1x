#ifndef INST_H
#define INST_H

#include <stdint.h>
#define N 256
inline uint32_t get_part(uint32_t instr, int from, int cnt) {
  return (instr >> from) & ((1 << cnt) - 1);
}

inline uint32_t sign_extend(uint32_t value, unsigned bits) {
  if (value & (1 << (bits - 1))) {
    value |= ~((1 << bits) - 1);
  }
  return value;
}

typedef struct {
  uint32_t rs1;
  uint32_t rs2;
  uint32_t rd;
  uint32_t shamt;
  uint32_t funct3;
  uint32_t funct7;
} RType;

typedef struct {
  uint32_t rs;
  uint32_t rd;
  uint32_t funct;
  uint32_t imm;
} IType;

typedef struct {
  uint32_t funct;
  uint32_t rs1;
  uint32_t rs2;
  uint32_t imm;
} SType;

typedef struct {
  uint32_t rd;
  uint32_t imm;
} UType;

inline IType get_IType(uint32_t inst) {
  IType it;
  it.rd = get_part(inst, 7, 5);
  it.funct = get_part(inst, 12, 3);
  it.rs = get_part(inst, 15, 5);
  it.imm = get_part(inst, 20, 12);
  return it;
}

inline RType get_RType(uint32_t inst) {
  RType it;
  it.rd = get_part(inst, 7, 5);
  it.funct3 = get_part(inst, 12, 3);
  it.rs1 = get_part(inst, 15, 5);
  it.rs2 = get_part(inst, 20, 5);
  it.funct7 = get_part(inst, 25, 7);
  return it;
}

inline UType get_UType(uint32_t inst) {
  UType ut;
  ut.rd = get_part(inst, 7, 5);
  ut.imm = get_part(inst, 12, 20);
  return ut;
}

inline SType get_SType(uint32_t inst) {
  SType st;
  st.funct = get_part(inst, 12, 3);
  st.rs1 = get_part(inst, 15, 5);
  st.rs2 = get_part(inst, 20, 5);
  st.imm = (get_part(inst, 25, 7) << 5) | get_part(inst, 7, 5);
  return st;
}

#endif // INST_H