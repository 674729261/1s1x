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

class __CSR_test() extends Module {
  val io = IO(new Bundle {
    val csr = Input(UInt(12.W))
    val wen = Input(Bool())
    val wdata = Input(UInt(32.W))
    val rdata = Output(UInt(32.W))

    val mepc = Output(UInt(32.W))
    val mtvec = Output(UInt(32.W))

    val cur_pc = Input(UInt(32.W))
    val new_cause = Input(UInt(32.W))
    val interruption = Input(Bool())
  })

  val b = Module(new CSR)
  b.io <> io
}

class CSRTester extends AnyFlatSpec {
  val random = new Random(12345)

  behavior of "CSR"
  it should "work correctly" in {
    simulate(new __CSR_test) { dut =>
      dut.reset.poke(true.B)
      dut.clock.step()
      dut.reset.poke(false.B)
      dut.io.wen.poke(false.B)
      dut.io.csr.poke(0xf11.U(12.W))
      dut.io.rdata.expect(0x79737978.U(32.W))
      dut.io.csr.poke(0xf12.U(12.W))
      dut.io.rdata.expect(0x17eb198.U(32.W))
      dut.io.csr.poke(0x305.U(12.W))
      dut.io.wdata.poke("hdeadbeef".U(32.W))
      dut.io.wen.poke(true.B)
      dut.clock.step()
      dut.io.wen.poke(false.B)
      dut.io.csr.poke(0xb00.U(12.W))
      dut.clock.step()
      dut.io.rdata.expect(2.U(32.W))
      dut.clock.step()
      dut.io.rdata.expect(3.U(32.W))
      dut.clock.step()
      dut.io.rdata.expect(4.U(32.W))
      dut.io.csr.poke(0x305.U(12.W))
      dut.io.rdata.expect("hdeadbeef".U(32.W))
    }
  }
}
