/*
 * Dummy file to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chisel3.util._

class RNG(WIDTH: Width = 8.W, XORWIDTH: Int = 4)
    extends Module
    with RequireAsyncReset {
  val io = IO(new Bundle {
    val load = Input(UInt(1.W))
    val load_data = Input(UInt(WIDTH))
    val en = Input(UInt(1.W))
    val out = Output(UInt(WIDTH))
  })

  val nxt = Wire(UInt(WIDTH))
  val reg = RegEnable(nxt, 0.U(WIDTH), io.en.asBool)

  when(io.load.asBool) { nxt := io.load_data }
    .otherwise { nxt := Cat(reg(XORWIDTH - 1, 0).xorR, reg(7, 1)) }

  io.out := reg

}

object AddMain extends App {
  println("Generating the RNG hardware")
  emitVerilog(
    new RNG(8.W, 4),
    Array("--target-dir", "svsrc")
  )
}
