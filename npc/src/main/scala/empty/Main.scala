package empty

import circt.stage._
import chisel3._
import chisel3.layer._

object AddMain extends App {
  println("Generating the CPU RTL")
  ChiselStage.emitSystemVerilogFile(
    // new CPU(init_pc = "h80000000".U(32.W)),
    // new CPU(init_pc = "h80000000".U(32.W)),
    new npc_top,
    Array(
      "--target-dir",
      "generated_svsrc"
    ),
    Array(
      "--disable-all-randomization",
      // "--disable-layers=Verification",
      "--lowering-options=" + List(
        // make yosys happy
        // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
        "disallowLocalVariables",
        // "disallowPackedArrays",
        "locationInfoStyle=wrapInAtSquareBracket"
      ).reduce(_ + "," + _)
    )
  )

  ChiselStage.emitSystemVerilogFile(
    new ALU(WIDTH = 4),
    Array(
      "--target-dir",
      "generated_svsrc/ALU4BIT"
    ),
    Array(
      "--disable-all-randomization",
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

  ChiselStage.emitSystemVerilogFile(
    new ysyx_25080216,
    Array(
      "--target-dir",
      "generated_svsrc/ysyx_25080216"
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
