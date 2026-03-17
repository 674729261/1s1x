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

class GPRTester extends AnyFlatSpec {
  val random = new Random(12345)
  val test_cases = 16384
  var arr = Array.ofDim[Long](32, test_cases)
  var is_write_arr = Array.ofDim[Int](test_cases)
  var wdata_arr = Array.ofDim[Long](test_cases)
  var waddr_arr = Array.ofDim[Int](test_cases)
  for (i <- 1 until test_cases) {
    val which_rreg1 = random.between(0, 32)
    val which_rreg2 = random.between(0, 32)
    val which_wreg = random.between(0, 32)
    val wdata = random.between(0, 0x100000000L)
    val is_write = random.between(0, 2)

    waddr_arr(i) = which_wreg
    is_write_arr(i) = is_write
    wdata_arr(i) = wdata

    for (j <- 1 until 32)
      arr(j)(i) = arr(j)(i - 1)

    if (is_write == 1 && which_wreg != 0)
      arr(which_wreg)(i) = wdata
  }

  behavior of "GPR"
  it should "work correctly" in {
    simulate(new GPR(32, 32)) { dut =>
      dut.io.wen.poke(true.B)
      for (i <- 0 until 32) {
        dut.io.raddr1.poke(i.U(5.W))
        dut.io.raddr2.poke(i.U(5.W))
        dut.io.waddr.poke(i.U(5.W))
        dut.io.wdata.poke(0.U(32.W))
        dut.clock.step()
        dut.io.rdata1.expect(0.U(32.W))
        dut.io.rdata2.expect(0.U(32.W))
      }

      for (i <- 1 until test_cases) {
        val which_rreg1 = random.between(0, 32)
        val which_rreg2 = random.between(0, 32)

        dut.io.wen.poke(is_write_arr(i).B)
        dut.io.raddr1.poke(which_rreg1.U(5.W))
        dut.io.raddr2.poke(which_rreg2.U(5.W))
        dut.io.waddr.poke(waddr_arr(i).U(5.W))
        dut.io.wdata.poke(wdata_arr(i).U(32.W))
        dut.clock.step()

        dut.io.rdata1.expect(arr(which_rreg1)(i).U(32.W))
        dut.io.rdata2.expect(arr(which_rreg2)(i).U(32.W))

      }
    }
  }
}
