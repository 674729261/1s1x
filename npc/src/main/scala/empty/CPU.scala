/*
 * Dummy file to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chisel3.util._

class CPU extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val instr = Input(UInt(32.W))
    val pc = Output(UInt(32.W))
  })

  def signExt32(in: UInt): UInt = {
    val input_width = in.getWidth
    Cat(Fill(32 - input_width, in(input_width - 1)), in)
  }

  val static_pc_next = Wire(UInt(32.W))
  val dynamic_pc_next = Wire(UInt(32.W))
  val pc = RegNext(next = dynamic_pc_next, init = 0.U(32.W))
  val is_addi = (io.instr(6, 0) === "b0010011".U(7.W))
  val is_jalr = (io.instr(6, 0) === "b1100111".U(7.W))
  io.pc := pc
  static_pc_next := pc + 4.U(32.W)

  val adder = Module(new Adder(32))
  dynamic_pc_next := Mux(is_jalr, adder.io.out, static_pc_next)

  val i_decoded = io.instr.asTypeOf(new IType)
  printf(p"${Hexadecimal(pc)}\n")
  val imm32_I = signExt32(i_decoded.imm12)

  val gpr = Module(new GPR)

  gpr.io.waddr := i_decoded.rd
  gpr.io.wen := true.B
  gpr.io.raddr1 := i_decoded.rs
  gpr.io.raddr2 := DontCare
  gpr.io.wdata := Mux(is_addi, adder.io.out, static_pc_next)

  adder.io.A := gpr.io.rdata1
  adder.io.B := imm32_I

}

object AddMain extends App {
  println("Generating the CPU RTL")
  emitVerilog(
    new CPU,
    Array("--target-dir", "svsrc")
  )
}
