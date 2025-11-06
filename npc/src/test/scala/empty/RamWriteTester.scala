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

class __RamWriteTester_test() extends Module {
  val io = IO(new Bundle {
    val word = Input(UInt(32.W))
    val lower2bit = Input(UInt(2.W))
    val is_word = Input(Bool())
    val is_half = Input(Bool())
    val is_byte = Input(Bool())

    val out = Output(UInt(32.W))
    val mask = Output(UInt(4.W))
  })

  val test = IO(new Bundle {
    val b0 = Output(UInt(8.W))
    val b1 = Output(UInt(8.W))
    val b2 = Output(UInt(8.W))
    val b3 = Output(UInt(8.W))
  })

  val r = Module(new RamWriteData)
  r.io <> io
  test.b0 := io.out(7, 0)
  test.b1 := io.out(15, 8)
  test.b2 := io.out(23, 16)
  test.b3 := io.out(31, 24)
}

class RamWriteTester extends AnyFlatSpec {
  behavior of "RamWriter"
  it should "work correctly" in {
    simulate(new __RamWriteTester_test) { dut =>
      dut.io.word.poke("h12345678".U(32.W))
      dut.io.is_word.poke(true.B)
      dut.io.out.expect("h12345678".U(32.W))
      dut.io.mask.expect("b1111".U(4.W))

      dut.io.is_word.poke(false.B)
      dut.io.is_half.poke(true.B)
      dut.io.lower2bit.poke("b00".U(2.W))
      dut.test.b0.expect("h78".U(8.W))
      dut.test.b1.expect("h56".U(8.W))
      dut.io.mask.expect("b0011".U(4.W))
      dut.io.lower2bit.poke("b10".U(2.W))
      dut.test.b2.expect("h78".U(8.W))
      dut.test.b3.expect("h56".U(8.W))
      dut.io.mask.expect("b1100".U(4.W))

      dut.io.is_half.poke(false.B)
      dut.io.is_byte.poke(true.B)
      dut.io.lower2bit.poke("b00".U(2.W))
      dut.test.b0.expect("h78".U(8.W))
      dut.io.mask.expect("b0001".U(4.W))
      dut.io.lower2bit.poke("b01".U(2.W))
      dut.test.b1.expect("h78".U(8.W))
      dut.io.mask.expect("b0010".U(4.W))
      dut.io.lower2bit.poke("b10".U(2.W))
      dut.test.b2.expect("h78".U(8.W))
      dut.io.mask.expect("b0100".U(4.W))
      dut.io.lower2bit.poke("b11".U(2.W))
      dut.test.b3.expect("h78".U(8.W))
      dut.io.mask.expect("b1000".U(4.W))
    }
  }
}
