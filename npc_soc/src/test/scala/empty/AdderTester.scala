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

class __Adder_test(WIDTH: Int) extends Module {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val Cin = Input(Bool())
    val out = Output(UInt(WIDTH.W))
    val Cout = Output(Bool())
    val overflow = Output(Bool())
  })

  val adder = Module(new Adder(WIDTH))
  adder.io <> io
}

class AdderTester extends AnyFlatSpec {
  val random = new Random(12345)
  val test_cases = 1024

  behavior of "Adder"
  it should "work correctly" in {
    simulate(new __Adder_test(32)) { dut =>
      dut.io.A.poke(0xffffffffL.U(32.W))
      dut.io.B.poke(0.U(32.W))
      dut.io.Cin.poke(false.B)
      dut.io.out.expect(0xffffffffL.U(32.W))
      dut.io.Cout.expect(false.B)
      dut.io.overflow.expect(false.B)

      dut.io.A.poke(0xffffffffL.U(32.W))
      dut.io.B.poke(1.U(32.W))
      dut.io.Cin.poke(false.B)
      dut.io.out.expect(0.U(32.W))
      dut.io.Cout.expect(true.B)
      dut.io.overflow.expect(false.B)

      dut.io.A.poke(0xffffffffL.U(32.W))
      dut.io.B.poke(0.U(32.W))
      dut.io.Cin.poke(true.B)
      dut.io.out.expect(0.U(32.W))
      dut.io.Cout.expect(true.B)
      dut.io.overflow.expect(false.B)

      dut.io.A.poke(0x7fffffffL.U(32.W))
      dut.io.B.poke(0.U(32.W))
      dut.io.Cin.poke(true.B)
      dut.io.out.expect(0x80000000L.U(32.W))
      dut.io.Cout.expect(false.B)
      dut.io.overflow.expect(true.B)

      dut.io.A.poke(1.U(32.W))
      dut.io.B.poke(0x7fffffffL.U(32.W))
      dut.io.Cin.poke(true.B)
      dut.io.out.expect(0x80000001L.U(32.W))
      dut.io.Cout.expect(false.B)
      dut.io.overflow.expect(true.B)

      dut.io.A.poke(1.U(32.W))
      dut.io.B.poke(0xffffffffL.U(32.W))
      dut.io.Cin.poke(true.B)
      dut.io.out.expect(1.U(32.W))
      dut.io.Cout.expect(true.B)
      dut.io.overflow.expect(false.B)

      for (i <- 0 until test_cases) {
        val A = random.between(0L, 0x100000000L)
        val B = random.between(0L, 0x100000000L)
        val Cin = random.between(0L, 2L)
        dut.io.A.poke(A.U(32.W))
        dut.io.B.poke(B.U(32.W))
        dut.io.Cin.poke(Cin.B)

        val Sum = (A + B + Cin) % 0x100000000L;
        val Cout = (A + B + Cin) >= 0x100000000L;
        val a_signed = (if (A >= (1L << 31L)) A - (1L << 32L) else A)
        val b_signed = (if (B >= (1L << 31L)) B - (1L << 32L) else B)
        if (a_signed >= 0 && b_signed >= 0) {
          val overflow = (Sum >= (1L << 31L))
          dut.io.overflow.expect(overflow.B)
        } else if (a_signed < 0 && b_signed < 0) {
          val overflow = (Sum < (1L << 31L))
          dut.io.overflow.expect(overflow.B)
        } else dut.io.overflow.expect(false.B)
      }
    }
  }
}
