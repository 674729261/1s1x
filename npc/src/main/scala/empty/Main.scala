package empty

import chisel3._

object AddMain extends App {
  println("Generating the CPU RTL")
  emitVerilog(
    new CPU,
    Array("--target-dir", "generated_svsrc")
  )
}
