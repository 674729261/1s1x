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

class __RamLoadTester_test() extends Module {
  val io = IO(new Bundle {
    val word = Input(UInt(32.W))
    val lower2bit = Input(UInt(2.W))
    val is_word = Input(Bool())
    val is_half = Input(Bool())
    val is_byte = Input(Bool())
    val is_unsigned = Input(Bool())

    val out = Output(UInt(32.W))
  })

  val ramloader = Module(new RamLoadData)
  ramloader.io <> io
}

class RamLoaderTester extends AnyFlatSpec {
  behavior of "RamLoader"
  it should "work correctly" in {
    simulate(new __RamLoadTester_test) { dut =>
      dut.io.word.poke("h12345678".U(32.W))
      dut.io.is_word.poke(true.B)
      dut.io.out.expect("h12345678".U(32.W))

      dut.io.is_word.poke(false.B)
      dut.io.is_byte.poke(true.B)
      dut.io.lower2bit.poke("b00".U(2.W))
      dut.io.out.expect("h78".U(32.W))
      dut.io.lower2bit.poke("b01".U(2.W))
      dut.io.out.expect("h56".U(32.W))
      dut.io.lower2bit.poke("b10".U(2.W))
      dut.io.out.expect("h34".U(32.W))
      dut.io.lower2bit.poke("b11".U(2.W))
      dut.io.out.expect("h12".U(32.W))

      dut.io.is_byte.poke(false.B)
      dut.io.is_half.poke(true.B)
      dut.io.lower2bit.poke("b00".U(2.W))
      dut.io.out.expect("h5678".U(32.W))
      dut.io.lower2bit.poke("b10".U(2.W))
      dut.io.out.expect("h1234".U(32.W))

      dut.io.word.poke("h82845678".U(32.W))
      dut.io.is_half.poke(false.B)
      dut.io.is_byte.poke(true.B)
      dut.io.is_unsigned.poke(false.B)
      dut.io.lower2bit.poke("b00".U(2.W))
      dut.io.out.expect("h78".U(32.W))
      dut.io.lower2bit.poke("b01".U(2.W))
      dut.io.out.expect("h56".U(32.W))
      dut.io.lower2bit.poke("b10".U(2.W))
      dut.io.out.expect("hffffff84".U(32.W))
      dut.io.lower2bit.poke("b11".U(2.W))
      dut.io.out.expect("hffffff82".U(32.W))

      dut.io.is_byte.poke(false.B)
      dut.io.is_half.poke(true.B)
      dut.io.lower2bit.poke("b00".U(2.W))
      dut.io.out.expect("h5678".U(32.W))
      dut.io.lower2bit.poke("b10".U(2.W))
      dut.io.out.expect("hffff8284".U(32.W))
    }
  }
}
