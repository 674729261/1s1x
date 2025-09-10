package empty

import chisel3._
import chisel3.util._

class IType extends Bundle {
  val imm12 = UInt(12.W)
  val rs = UInt(5.W)
  val funct3 = UInt(3.W)
  val rd = UInt(5.W)
  val opcode = UInt(7.W)
}

class RType extends Bundle {
  val funct7 = UInt(7.W)
  val rs2 = UInt(5.W)
  val rs1 = UInt(5.W)
  val funct3 = UInt(3.W)
  val rd = UInt(5.W)
  val opcode = UInt(7.W)
}

class UType extends Bundle {
  val imm20 = UInt(20.W)
  val rd = UInt(5.W)
  val opcode = UInt(7.W)
}

class SType extends Bundle {
  val imm7U = UInt(7.W)
  val rs2 = UInt(5.W)
  val rs1 = UInt(5.W)
  val funct3 = UInt(3.W)
  val imm5L = UInt(5.W)
  val opcode = UInt(7.W)
}

class DecodeInstr extends RawModule {
  val io = IO(new Bundle {
    val inst = Input(UInt(32.W))

    val rs1 = Output(UInt(5.W))
    val rs2 = Output(UInt(5.W))
    val rd = Output(UInt(5.W))
    val funct3 = Output(UInt(3.W))
    val funct7 = Output(UInt(7.W))
    val imm = Output(UInt(32.W))

    val is_arithmetic_imm = Output(Bool())
    val is_arithmetic_reg = Output(Bool())
    val is_store = Output(Bool())
    val is_load = Output(Bool())
    val is_branch = Output(Bool())
    val is_jal = Output(Bool())
    val is_jalr = Output(Bool())
    val is_lui = Output(Bool())
    val is_auipc = Output(Bool())
    val is_ebreak = Output(Bool())

    val is_alu_a_pc = Output(Bool())
    val is_alu_b_imm = Output(Bool())
    val is_alu_force_add = Output(Bool())
    val is_gpr_write = Output(Bool())
  })
  val inst = io.inst;

  io.is_arithmetic_imm := (inst(6, 0) === "b0010011".U(7.W))
  io.is_arithmetic_reg := (inst(6, 0) === "b0110011".U(7.W))
  io.is_store := (inst(6, 0) === "b0100011".U(7.W))
  io.is_load := (inst(6, 0) === "b0000011".U(7.W))
  io.is_branch := (inst(6, 0) === "b1100011".U(7.W))
  io.is_jal := (inst(6, 0) === "b1101111".U(7.W))
  io.is_jalr := (inst(6, 0) === "b1100111".U(7.W))
  io.is_lui := (inst(6, 0) === "b0110111".U(7.W))
  io.is_auipc := (inst(6, 0) === "b0010111".U(7.W))
  io.is_ebreak := (inst(6, 0) === "b1110011".U(7.W))

  val is_I = io.is_arithmetic_imm || io.is_ebreak || io.is_load || io.is_jalr
  val is_R = io.is_arithmetic_reg
  val is_S = io.is_store
  val is_B = io.is_branch
  val is_J = io.is_jal
  val is_U = io.is_auipc || io.is_lui

  io.is_alu_a_pc := is_B || is_J || io.is_auipc
  io.is_alu_b_imm := is_I || is_B || is_S
  io.is_alu_force_add := io.is_store || io.is_load || io.is_branch
  io.is_gpr_write := io.is_jal || io.is_jalr || is_U || is_R ||
    io.is_arithmetic_imm || io.is_arithmetic_reg || io.is_load

  io.rs1 := inst(19, 15)
  io.rs2 := inst(24, 20)
  io.rd := inst(11, 7)
  io.funct3 := inst(14, 12)
  io.funct7 := inst(31, 25)

  when(is_B) {
    io.imm := Cat(
      Fill(20, inst(31)),
      inst(7),
      inst(30, 25),
      inst(11, 8),
      0.U(1.W)
    )
  }.elsewhen(is_S) {
    io.imm := Cat(
      Fill(20, inst(31)),
      inst(31, 25),
      inst(11, 7)
    )
  }.elsewhen(is_J) {
    io.imm := Cat(
      Fill(12, inst(31)),
      inst(19, 12),
      inst(20),
      inst(30, 21),
      0.U(1.W)
    )
  }.elsewhen(is_U) {
    io.imm := Cat(
      inst(31, 12),
      0.U(12.W)
    )
  }.otherwise { // I_type
    io.imm := Cat(
      Fill(20, inst(31)),
      inst(31, 20)
    )
  }
}
