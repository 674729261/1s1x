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
import empty.Branch

class CSRTester extends AnyFlatSpec {
  val random = new Random(12345)

  behavior of "CSR"
  it should "work correctly" in {
    simulate(new CSR) { dut =>
      dut.io.ok_to_step.poke(true.B)
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
      dut.io.mtvec.expect("hdeadbeef".U(32.W))

      dut.io.interruption.poke(true.B)
      dut.io.new_cause.poke(0x12345678.U(32.W))
      dut.io.cur_pc.poke(0x8000ff0.U(32.W))
      dut.clock.step()
      dut.io.csr.poke(0x342.U(12.W))
      dut.io.rdata.expect(0x12345678.U(32.W))
      dut.io.csr.poke(0x341.U(12.W))
      dut.io.rdata.expect(0x8000ff0.U(32.W))
      dut.io.mepc.expect(0x8000ff0.U(32.W))
    }
  }
}
