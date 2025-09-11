/*
 * Dummy tester to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chiseltest._
import org.scalatest.flatspec.AnyFlatSpec

import scala.util.Random

class __DecoderInstr_test() extends Module {
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

  val alu = Module(new DecodeInstr)
  alu.io <> io
}

class DecoderTester extends AnyFlatSpec with ChiselScalatestTester {
  behavior of "DecodeInstr"
  it should "work correctly" in {
    test(new __DecoderInstr_test) { dut =>
      // addi
      dut.io.inst.poke("b101010101100_00111_000_11110_0010011".U(32.W))

      dut.io.imm.expect("b11111111111111111111101010101100".U(32.W))
      dut.io.is_arithmetic_imm.expect(true.B)
      dut.io.rs1.expect("b00111".U(5.W))
      dut.io.rd.expect("b11110".U(5.W))
      dut.io.funct3.expect("b000".U(3.W))

      // xori
      dut.io.inst.poke("b001010101100_10110_100_00010_0010011".U(32.W))

      dut.io.imm.expect("b00000000000000000000001010101100".U(32.W))
      dut.io.is_arithmetic_imm.expect(true.B)
      dut.io.rs1.expect("b10110".U(5.W))
      dut.io.rd.expect("b00010".U(5.W))
      dut.io.funct3.expect("b100".U(3.W))

      // xori
      dut.io.inst.poke("b001010101100_10110_100_00010_0010011".U(32.W))

      dut.io.imm.expect("b00000000000000000000001010101100".U(32.W))
      dut.io.is_arithmetic_imm.expect(true.B)
      dut.io.rs1.expect("b10110".U(5.W))
      dut.io.rd.expect("b00010".U(5.W))
      dut.io.funct3.expect("b100".U(3.W))

      // beq
      dut.io.inst.poke("b1111101_10000_00000_000_01110_1100011".U(32.W))

      dut.io.imm.expect("b11111111111111111111011110101110".U(32.W))
      dut.io.rs1.expect("b00000".U(5.W))
      dut.io.rs2.expect("b10000".U(5.W))
      dut.io.is_branch.expect(true.B)
      dut.io.funct3.expect("b000".U(3.W))

      // sra
      dut.io.inst.poke("b0100000_11011_10001_101_00001_0110011".U(32.W))

      dut.io.rs1.expect("b10001".U(5.W))
      dut.io.rs2.expect("b11011".U(5.W))
      dut.io.is_arithmetic_reg.expect(true.B)
      dut.io.funct3.expect("b101".U(3.W))

      // lui
      dut.io.inst.poke("b10101010101010101010_10101_0110111".U(32.W))

      dut.io.rd.expect("b10101".U(5.W))
      dut.io.is_lui.expect(true.B)
      dut.io.imm.expect("b10101010101010101010_000000000000".U(32.W))

      // jal
      dut.io.inst.poke("b10101011110011011110_00110_1101111".U(32.W))

      dut.io.rd.expect("b00110".U(5.W))
      dut.io.is_jal.expect(true.B)
      dut.io.imm.expect("b11111111111111011110001010111100".U(32.W))

      // jalr
      dut.io.inst.poke("b101010101010_10101_000_01010_1100111".U(32.W))

      dut.io.rs1.expect("b10101".U(5.W))
      dut.io.rd.expect("b01010".U(5.W))
      dut.io.is_jalr.expect(true.B)
      dut.io.imm.expect("b11111111111111111111101010101010".U(32.W))
    }
  }
}
