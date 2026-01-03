/*
 * Dummy tester to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chisel3.simulator.EphemeralSimulator._
import org.scalatest.flatspec.AnyFlatSpec

import scala.util.Random
import empty.IDU

class IDUTester extends AnyFlatSpec {
  behavior of "IDU"
  it should "work correctly" in {
    simulate(new IDU) { dut =>
      // addi
      dut.in.bits.inst.poke("b101010101100_00111_000_11110_0010011".U(32.W))
      dut.out.bits.fields.imm
        .expect("b11111111111111111111101010101100".U(32.W))
      dut.out.bits.fields.rs1.expect("b00111".U(5.W))
      dut.out.bits.fields.rd.expect("b11110".U(5.W))
      dut.out.bits.itype.is_arithmetic_imm.expect(true.B)
      dut.out.bits.fields.funct3.expect("b000".U(3.W))

      // xori
      dut.in.bits.inst.poke("b001010101100_10110_100_00010_0010011".U(32.W))
      dut.out.bits.fields.imm
        .expect("b00000000000000000000001010101100".U(32.W))
      dut.out.bits.fields.rs1.expect("b10110".U(5.W))
      dut.out.bits.fields.rd.expect("b00010".U(5.W))
      dut.out.bits.itype.is_arithmetic_imm.expect(true.B)
      dut.out.bits.fields.funct3.expect("b100".U(3.W))

      // xori
      dut.in.bits.inst.poke("b001010101100_10110_100_00010_0010011".U(32.W))
      dut.out.bits.fields.imm
        .expect("b00000000000000000000001010101100".U(32.W))
      dut.out.bits.fields.rs1.expect("b10110".U(5.W))
      dut.out.bits.fields.rd.expect("b00010".U(5.W))
      dut.out.bits.itype.is_arithmetic_imm.expect(true.B)
      dut.out.bits.fields.funct3.expect("b100".U(3.W))

      // beq
      dut.in.bits.inst.poke("b1111101_10000_00000_000_01110_1100011".U(32.W))
      dut.out.bits.fields.imm
        .expect("b11111111111111111111011110101110".U(32.W))
      dut.out.bits.fields.rs1.expect("b00000".U(5.W))
      dut.out.bits.fields.rs2.expect("b10000".U(5.W))
      dut.out.bits.itype.is_branch.expect(true.B)
      dut.out.bits.fields.funct3.expect("b000".U(3.W))

      // sra
      dut.in.bits.inst.poke("b0100000_11011_10001_101_00001_0110011".U(32.W))
      dut.out.bits.fields.rs1.expect("b10001".U(5.W))
      dut.out.bits.fields.rs2.expect("b11011".U(5.W))
      dut.out.bits.itype.is_arithmetic_reg.expect(true.B)
      dut.out.bits.fields.funct3.expect("b101".U(3.W))

      // lui
      dut.in.bits.inst.poke("b10101010101010101010_10101_0110111".U(32.W))
      dut.out.bits.fields.rd.expect("b10101".U(5.W))
      dut.out.bits.itype.is_lui.expect(true.B)
      dut.out.bits.fields.imm
        .expect("b10101010101010101010_000000000000".U(32.W))

      // jal
      dut.in.bits.inst.poke("b10101011110011011110_00110_1101111".U(32.W))
      dut.out.bits.fields.rd.expect("b00110".U(5.W))
      dut.out.bits.itype.is_jal.expect(true.B)
      dut.out.bits.fields.imm
        .expect("b11111111111111011110001010111100".U(32.W))

      // jalr
      dut.in.bits.inst.poke("b101010101010_10101_000_01010_1100111".U(32.W))
      dut.out.bits.fields.rs1.expect("b10101".U(5.W))
      dut.out.bits.fields.rd.expect("b01010".U(5.W))
      dut.out.bits.itype.is_jalr.expect(true.B)
      dut.out.bits.fields.imm
        .expect("b11111111111111111111101010101010".U(32.W))

      // csrrw
      dut.in.bits.inst.poke("b101010101010_10101_001_01010_1110011".U(32.W))
      dut.out.bits.fields.rs1.expect("b10101".U(5.W))
      dut.out.bits.fields.rd.expect("b01010".U(5.W))
      dut.out.bits.itype.is_csrop.expect(true.B)
      dut.out.bits.controls.is_csr_visit.expect(true.B)
      dut.out.bits.controls.is_csr_masked.expect(false.B)
      dut.out.bits.fields.csr.expect("b101010101010".U(12.W))

      // csrrs
      dut.in.bits.inst.poke("b101010101010_10101_010_01010_1110011".U(32.W))
      dut.out.bits.fields.rs1.expect("b10101".U(5.W))
      dut.out.bits.fields.rd.expect("b01010".U(5.W))
      dut.out.bits.itype.is_csrop.expect(true.B)
      dut.out.bits.controls.is_csr_visit.expect(true.B)
      dut.out.bits.controls.is_csr_masked.expect(true.B)
      dut.out.bits.fields.csr.expect("b101010101010".U(12.W))

      // jalr zero, t1, -4
      dut.in.bits.inst.poke("hffc30067".U(32.W))
      dut.out.bits.fields.rs1.expect("b00110".U(5.W))
      dut.out.bits.fields.rd.expect("b00000".U(5.W))
      dut.out.bits.itype.is_jalr.expect(true.B)
      dut.out.bits.fields.imm
        .expect("b11111111111111111111111111111100".U(32.W))
      dut.out.bits.itype.is_csrop.expect(false.B)
      dut.out.bits.itype.is_branch.expect(false.B)
      dut.out.bits.controls.is_alu_a_pc.expect(false.B)
      dut.out.bits.controls.is_alu_b_reg.expect(false.B)
      dut.out.bits.itype.is_jal.expect(false.B)
    }
  }
}
