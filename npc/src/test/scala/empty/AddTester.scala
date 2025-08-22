/*
 * Dummy tester to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chiseltest._
import org.scalatest.flatspec.AnyFlatSpec

class AddTester extends AnyFlatSpec with ChiselScalatestTester {

  "Add" should "work" in {
    test(new rng) { dut =>
      dut.io.load_data.poke("hff".U(8.W))
      dut.io.load.poke(1.U)
      dut.io.en.poke(1.U)
      dut.clock.step(1)
      dut.io.load.poke(0.U)
      for (a <- 0 until 128) {
        dut.clock.step(1)
        val v = dut.io.out.peek()
        println(v)
        // for (b <- 0 to 3) {
        //   val result = a + b
        //   dut.io.a.poke(a.U)
        //   dut.io.b.poke(b.U)
        //   dut.clock.step(1)
        //   dut.io.c.expect(result.U)
        // }
      }
    }
  }
}
