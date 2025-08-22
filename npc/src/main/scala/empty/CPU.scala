/*
 * Dummy file to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chisel3.util._

class Memory extends BlackBox {
  val io = IO(new Bundle {
    val valid = Input(Bool())
    val wen = Input(Bool())
    val waddr = Input(UInt(32.W))
    val raddr = Input(UInt(32.W))
    val wmask = Input(UInt(4.W))
    val wdata = Input(UInt(32.W))
    val rdata = Output(UInt(32.W))
  })
}

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

  io.pc := pc
  static_pc_next := pc + 4.U(32.W)

  val adder = Module(new Adder(32))
  dynamic_pc_next := static_pc_next

  val i_decoded = io.instr.asTypeOf(new IType)
  val r_decoded = io.instr.asTypeOf(new RType)
  val u_decoded = io.instr.asTypeOf(new UType)
  val immI = signExt32(i_decoded.imm12)
  val immU = Cat(u_decoded.imm20, 0.U(12.W))
  val rs1 = r_decoded.rs1
  val rs2 = r_decoded.rs2
  val rd = r_decoded.rd

  val gpr = Module(new GPR)

  gpr.io.waddr := rd
  gpr.io.wen := true.B
  gpr.io.raddr1 := rs1
  gpr.io.raddr2 := rs2
  gpr.io.wdata := adder.io.out
  adder.io.A := gpr.io.rdata1
  adder.io.B := gpr.io.rdata2
  switch(io.instr(6, 0)) {
    is("b0010011".U(7.W)) { // addi
      adder.io.B := immI
    }
    is("b0110011".U(7.W)) { // add

    }
    is("b1100111".U(7.W)) { // jalr
      adder.io.B := immI
      dynamic_pc_next := Cat(adder.io.out(31, 1), 0.U(1.W))
      gpr.io.wdata := static_pc_next
    }
    is("b0110111".U(7.W)) { // lui
      gpr.io.wdata := immU
    }
  }

}
