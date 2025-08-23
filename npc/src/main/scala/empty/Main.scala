package empty

import chisel3._

object AddMain extends App {
  println("Generating the CPU RTL")
  emitVerilog(
    new CPU("h00000000".U(32.W)),
    Array("--target-dir", "generated_svsrc")
  )
}
