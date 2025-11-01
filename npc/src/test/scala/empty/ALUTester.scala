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

class __ALU_test(WIDTH: Int) extends Module {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val funct3 = Input(UInt(3.W))
    val is_sub_sra = Input(Bool())
    val is_force_add = Input(Bool())
    val out = Output(UInt(WIDTH.W))

  })

  val alu = Module(new ALU(WIDTH))
  alu.io <> io
}

class ALUTester extends AnyFlatSpec {
  def set_input(
      dut: __ALU_test,
      a: UInt,
      b: UInt,
      funct3: UInt,
      is_sub_sra: Bool,
      is_force_add: Bool
  ) = {
    dut.io.A.poke(a)
    dut.io.B.poke(b)
    dut.io.funct3.poke(funct3)
    dut.io.is_sub_sra.poke(is_sub_sra)
    dut.io.is_force_add.poke(is_force_add)
  }

  val random = new Random(12345)
  val test_cases = 128
  behavior of "ALU"
  it should "work correctly" in {
    simulate(new __ALU_test(32)) { dut =>
      for (i <- 0 until test_cases) { // add
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(1L << 32)
        val expected = (a + b) % (1L << 32)

        set_input(dut, a.U(32.W), b.U(32.W), "b000".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
        set_input(
          dut,
          a.U(32.W),
          b.U(32.W),
          random.nextLong(8).U(3.W),
          false.B,
          true.B
        )

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // sub
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(1L << 32)
        val expected = (a - b + (1L << 32)) % (1L << 32)

        set_input(dut, a.U(32.W), b.U(32.W), "b000".U(3.W), true.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // sltu
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(1L << 32)
        val expected = if (a < b) 1 else 0

        set_input(dut, a.U(32.W), b.U(32.W), "b011".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // slt
        var a: Long = random.nextLong(1L << 32)
        var b: Long = random.nextLong(1L << 32)
        if (a >= (1L << 31)) a -= (1L << 32)
        if (b >= (1L << 31)) b -= (1L << 32)
        val expected = if (a < b) 1 else 0
        a = (a + (1L << 32)) % (1L << 32)
        b = (b + (1L << 32)) % (1L << 32)

        set_input(dut, a.U(32.W), b.U(32.W), "b010".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // and
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(1L << 32)
        val expected = a & b

        set_input(dut, a.U(32.W), b.U(32.W), "b111".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // or
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(1L << 32)
        val expected = a | b

        set_input(dut, a.U(32.W), b.U(32.W), "b110".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // xor
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(1L << 32)
        val expected = a ^ b

        set_input(dut, a.U(32.W), b.U(32.W), "b100".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // sll
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(32)
        val expected = (a << b) % (1L << 32)

        set_input(dut, a.U(32.W), b.U(32.W), "b001".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // srl
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(32)
        val expected = (a >> b)

        set_input(dut, a.U(32.W), b.U(32.W), "b101".U(3.W), false.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }
      for (i <- 0 until test_cases) { // sra
        val a: Long = random.nextLong(1L << 32)
        val b: Long = random.nextLong(32)
        val expected =
          if (a < (1L << 31)) (a >> b)
          else (a >> b) | ((1L << 32) - (1L << (32 - b)))

        set_input(dut, a.U(32.W), b.U(32.W), "b101".U(3.W), true.B, false.B)

        dut.io.out.expect(expected.U(32.W))
      }

    }
  }
}
