package empty

import chisel3._

object AddMain extends App {
  println("Generating the CPU RTL")
  emitVerilog(
    new CPU(init_pc = "h80000000".U(32.W)),
    Array("--target-dir", "generated_svsrc")
  )
}
