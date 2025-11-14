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
import _root_.empty.empty.Branch

class __Branch_test(WIDTH: Int) extends Module {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val funct3 = Input(UInt(3.W))
    val jump = Output(UInt(WIDTH.W))

  })

  val b = Module(new Branch(WIDTH))
  b.io <> io
}

class BranchTester extends AnyFlatSpec {
  val random = new Random(12345)
  val test_cases = 128
  behavior of "Branch"
  it should "work correctly" in {
    simulate(new __Branch_test(32)) { dut =>
      dut.io.A.poke("h80000000".U(32.W))
      dut.io.B.poke("h80000000".U(32.W))
      dut.io.funct3.poke("b000".U(3.W))
      dut.io.jump.expect(true.B)
      dut.io.funct3.poke("b001".U(3.W))
      dut.io.jump.expect(false.B)
      dut.io.funct3.poke("b100".U(3.W))
      dut.io.jump.expect(false.B)
      dut.io.funct3.poke("b101".U(3.W))
      dut.io.jump.expect(true.B)
      dut.io.funct3.poke("b110".U(3.W))
      dut.io.jump.expect(false.B)
      dut.io.funct3.poke("b111".U(3.W))
      dut.io.jump.expect(true.B)

      dut.io.A.poke("h7FFFFFFF".U(32.W))
      dut.io.B.poke("h80000000".U(32.W))
      dut.io.funct3.poke("b000".U(3.W))
      dut.io.jump.expect(false.B)
      dut.io.funct3.poke("b001".U(3.W))
      dut.io.jump.expect(true.B)
      dut.io.funct3.poke("b100".U(3.W))
      dut.io.jump.expect(false.B)
      dut.io.funct3.poke("b101".U(3.W))
      dut.io.jump.expect(true.B)
      dut.io.funct3.poke("b110".U(3.W))
      dut.io.jump.expect(true.B)
      dut.io.funct3.poke("b111".U(3.W))
      dut.io.jump.expect(false.B)

      for (i <- 0 until test_cases) {
        val A = random.nextLong(1L << 32)
        val B = random.nextLong(1L << 32)
        dut.io.A.poke(A.U(32.W))
        dut.io.B.poke(B.U(32.W))

        val res_eq = A == B
        val res_ne = A != B
        val res_ltu = A < B
        val res_geu = A >= B
        val As = if (A >= (1L << 31)) A - (1L << 32) else A
        val Bs = if (B >= (1L << 31)) B - (1L << 32) else B
        val res_lt = As < Bs
        val res_ge = As >= Bs

        dut.io.funct3.poke("b000".U(3.W))

        dut.io.jump.expect(res_eq.B)
        dut.io.funct3.poke("b001".U(3.W))

        dut.io.jump.expect(res_ne.B)
        dut.io.funct3.poke("b100".U(3.W))

        dut.io.jump.expect(res_lt.B)
        dut.io.funct3.poke("b101".U(3.W))

        dut.io.jump.expect(res_ge.B)
        dut.io.funct3.poke("b110".U(3.W))

        dut.io.jump.expect(res_ltu.B)
        dut.io.funct3.poke("b111".U(3.W))

        dut.io.jump.expect(res_geu.B)

        dut.io.A.poke(A.U(32.W))
        dut.io.B.poke(A.U(32.W))
        dut.io.funct3.poke("b000".U(3.W))

        dut.io.jump.expect(true.B)
        dut.io.funct3.poke("b001".U(3.W))

        dut.io.jump.expect(false.B)
      }

    }
  }
}
