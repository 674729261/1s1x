package empty

import circt.stage._
import chisel3._
import chisel3.layer._

object AddMain extends App {
  println("Generating the CPU RTL")

  ChiselStage.emitSystemVerilogFile(
    new ysyx_25080216,
    Array(
      "--target-dir",
      "generated_svsrc"
    ),
    Array(
      "--disable-layers=Verification",
      "--lowering-options=" + List(
        // make yosys happy
        // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
        "disallowLocalVariables",
        // "disallowPackedArrays",
        "locationInfoStyle=wrapInAtSquareBracket"
      ).reduce(_ + "," + _)
    )
  )

}
