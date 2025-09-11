package empty

import chisel3._
import chisel3.util._

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
    val is_alu_b_reg = Output(Bool())
    val is_alu_sub_sra = Output(Bool())
    val is_alu_force_add = Output(Bool())
    val is_gpr_wdata_from_ram = Output(Bool())
    val is_gpr_wdata_from_snpc = Output(Bool())
    val is_gpr_wdata_from_alu = Output(Bool())
    val is_gpr_wdata_from_imm = Output(Bool())

    val is_gpr_wen = Output(Bool())

    val is_ram_word = Output(Bool())
    val is_ram_half = Output(Bool())
    val is_ram_byte = Output(Bool())
    val is_load_unsigned = Output(Bool())

    val is_ram_valid = Output(Bool())
    val is_ram_wen = Output(Bool())
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
  io.is_alu_b_reg := is_R
  io.is_alu_sub_sra :=
    io.inst(5) && !(io.is_arithmetic_reg && io.funct3 === "b000".U(3.W))
  io.is_alu_force_add := io.is_store || io.is_load || io.is_branch
  io.is_gpr_wdata_from_ram := io.is_load
  io.is_gpr_wdata_from_snpc := io.is_jal || io.is_jalr
  io.is_gpr_wdata_from_imm := io.is_lui
  io.is_gpr_wdata_from_alu := (!io.is_gpr_wdata_from_imm) && (!io.is_gpr_wdata_from_ram) && (!io.is_gpr_wdata_from_snpc)
  io.is_gpr_wen := is_U || io.is_load || is_R || io.is_arithmetic_imm || io.is_jal || io.is_jalr

  io.is_ram_byte := (io.funct3(1, 0) === "b00".U(2.W))
  io.is_ram_word := (io.funct3(1, 0) === "b10".U(2.W))
  io.is_ram_half := (io.funct3(1, 0) === "b01".U(2.W))
  io.is_load_unsigned := io.funct3(2)

  io.is_ram_valid := io.is_load || io.is_store
  io.is_ram_wen := io.is_store

  io.rs1 := inst(19, 15)
  io.rs2 := inst(24, 20)
  io.rd := inst(11, 7)
  io.funct3 := inst(14, 12)
  io.funct7 := inst(31, 25)

  val imm_B = Wire(UInt(32.W))
  val imm_S = Wire(UInt(32.W))
  val imm_J = Wire(UInt(32.W))
  val imm_U = Wire(UInt(32.W))
  val imm_I = Wire(UInt(32.W))

  imm_B := Cat(
    Fill(20, inst(31)),
    inst(7),
    inst(30, 25),
    inst(11, 8),
    0.U(1.W)
  )
  imm_S := Cat(
    Fill(20, inst(31)),
    inst(31, 25),
    inst(11, 7)
  )
  imm_J := Cat(
    Fill(12, inst(31)),
    inst(19, 12),
    inst(20),
    inst(30, 21),
    0.U(1.W)
  )

  imm_U := Cat(
    inst(31, 12),
    0.U(12.W)
  )
  imm_I := Cat(
    Fill(20, inst(31)),
    inst(31, 20)
  )

  io.imm := Mux1H(
    Seq(
      is_B -> imm_B,
      is_S -> imm_S,
      is_J -> imm_J,
      is_U -> imm_U,
      is_I -> imm_I
    )
  )

}
