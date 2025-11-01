package empty

import circt.stage._
import chisel3._

object AddMain extends App {
  println("Generating the CPU RTL")
  ChiselStage.emitSystemVerilogFile(
    new CPU(init_pc = "h80000000".U(32.W)),
    Array(
      // "--target-dir",
      // "generated_svsrc/ALL_IN_ONE"
    ),
    Array(
      "--disable-all-randomization",
      "--split-verilog",
      "-o",
      "generated_svsrc"
    )
  )

}
