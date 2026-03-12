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

class FIFOTester extends AnyFlatSpec {
  behavior of "FIFO"
  it should "work correctly" in {
    simulate(new FIFO(BITWIDTH = 32, DEPTH_2POW = 3)) { dut =>
      dut.reset.poke(true.B)
      dut.io.push.poke(false.B)
      dut.io.pop.poke(false.B)
      dut.clock.step()
      dut.reset.poke(false.B)
      dut.clock.step()
      dut.io.empty.expect(true.B)
      dut.io.full.expect(false.B)
      dut.io.push.poke(true.B)
      dut.io.pop.poke(false.B)
      for (i <- 0 until 8) {
        dut.io.wdata.poke((i * 1234 + 456).U(32.W))
        dut.clock.step()
        dut.io.empty.expect(false.B)
        dut.io.full.expect((i == 7).B)
        dut.io.front.expect(456.U(32.W))
      }
      dut.io.push.poke(true.B)
      dut.io.pop.poke(true.B)
      for (i <- 0 until 8) {
        dut.io.wdata.poke((i * 2345 + 789).U(32.W))
        dut.io.front.expect((i * 1234 + 456).U(32.W))
        dut.clock.step()
        dut.io.empty.expect(false.B)
        dut.io.full.expect(true.B)
      }
      dut.io.push.poke(false.B)
      dut.io.pop.poke(true.B)
      for (i <- 0 until 8) {
        dut.io.front.expect((i * 2345 + 789).U(32.W))
        dut.clock.step()
        dut.io.empty.expect((i == 7).B)
        dut.io.full.expect(false.B)
      }
    }
  }
}
