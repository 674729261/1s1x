package empty

import circt.stage._
import chisel3._

object AddMain extends App {
  println("Generating the CPU RTL")
  ChiselStage.emitSystemVerilogFile(
    new CPU(init_pc = "h80000000".U(32.W)),
    Array(
      "--target-dir",
      "generated_svsrc"
    ),
    Array("--disable-all-randomization", "--disable-layers=Verification")
  )

  ChiselStage.emitSystemVerilogFile(
    new ALU(WIDTH = 4),
    Array(
      "--target-dir",
      "generated_svsrc/ALU4BIT"
    ),
    Array("--disable-all-randomization", "--disable-layers=Verification")
  )

}
