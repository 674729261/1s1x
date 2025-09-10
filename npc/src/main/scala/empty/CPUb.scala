/*
 * Dummy file to start a Chisel project.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package empty

import chisel3._
import chisel3.util._
import ujson.Arr
import upickle.default

class Memoryb extends BlackBox {
  val io = IO(new Bundle {
    val clk = Input(Bool())
    val valid = Input(Bool())
    val wen = Input(Bool())
    val waddr = Input(UInt(32.W))
    val raddr = Input(UInt(32.W))
    val wmask = Input(UInt(4.W))
    val wdata = Input(UInt(32.W))
    val rdata = Output(UInt(32.W))
  })
}

class Trapb extends BlackBox {
  val io = IO(new Bundle {
    val clk = Input(Bool())
    val ebreak = Input(Bool())
  })
}

class CPUb(init_pc: UInt) extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val instr = Input(UInt(32.W))
    val pc = Output(UInt(32.W))
  })

  def signExt32(in: UInt, bits: Int): UInt = {
    Cat(Fill(32 - bits, in(bits - 1)), in)
  }

  val Trapper = Module(new Trap)
  Trapper.io.clk := clock.asBool
  Trapper.io.ebreak := false.B
  val static_pc_next = Wire(UInt(32.W))
  val dynamic_pc_next = Wire(UInt(32.W))
  val pc = RegNext(next = dynamic_pc_next, init = init_pc)

  io.pc := pc
  static_pc_next := pc + 4.U(32.W)

  val adder = Module(new Adder(32))
  dynamic_pc_next := static_pc_next

  val i_decoded = io.instr.asTypeOf(new IType)
  val r_decoded = io.instr.asTypeOf(new RType)
  val u_decoded = io.instr.asTypeOf(new UType)
  val s_decoded = io.instr.asTypeOf(new SType)
  val immI = signExt32(i_decoded.imm12, 12)
  val immU = Cat(u_decoded.imm20, 0.U(12.W))
  val immS = signExt32(Cat(s_decoded.imm7U, s_decoded.imm5L), 12)
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

  val mem_wdata = Wire(Vec(4, UInt(8.W)))
  mem_wdata := gpr.io.rdata2.asTypeOf(Vec(4, UInt(8.W)))
  val memory_proxy = Module(new Memory)
  memory_proxy.io.clk := clock.asBool
  memory_proxy.io.raddr := adder.io.out
  memory_proxy.io.valid := false.B
  memory_proxy.io.wen := false.B
  memory_proxy.io.waddr := adder.io.out
  memory_proxy.io.wdata := mem_wdata.asUInt
  memory_proxy.io.wmask := "b1111".U(4.W)

  switch(io.instr(6, 0)) {
    is("b1110011".U(7.W)) { // ebreak
      Trapper.io.ebreak := true.B
    }
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
    is("b0100011".U(7.W)) { // save
      memory_proxy.io.valid := true.B
      memory_proxy.io.wen := true.B
      adder.io.B := immS
      gpr.io.wen := false.B
      switch(s_decoded.funct3) {
        is("b010".U(3.W)) { // sw
        }
        is("b000".U(3.W)) { // sb
          val byte = gpr.io.rdata2(7, 0)
          switch(adder.io.out(1, 0)) {
            // is("b00".U) { write_data := Cat(0.U(24.W), byte) }
            // is("b01".U) { write_data := Cat(0.U(16.W), byte, 0.U(8.W)) }
            // is("b10".U) { write_data := Cat(0.U(8.W), byte, 0.U(16.W)) }
            // is("b11".U) { write_data := Cat(byte, 0.U(24.W)) }
            is("b00".U) { mem_wdata(0) := byte }
            is("b01".U) { mem_wdata(1) := byte }
            is("b10".U) { mem_wdata(2) := byte }
            is("b11".U) { mem_wdata(3) := byte }
          }
          memory_proxy.io.wmask := UIntToOH(adder.io.out(1, 0))
        }
      }
    }
    is("b0000011".U(7.W)) { // load
      adder.io.B := immI
      memory_proxy.io.raddr := adder.io.out
      memory_proxy.io.valid := true.B
      switch(i_decoded.funct3) {
        is("b010".U(3.W)) { // lw
          gpr.io.wdata := memory_proxy.io.rdata
        }
        is("b100".U(3.W)) { // lbu
          val read_data = memory_proxy.io.rdata
          switch(adder.io.out(1, 0)) {
            is("b00".U) { gpr.io.wdata := Cat(0.U(24.W), read_data(7, 0)) }
            is("b01".U) { gpr.io.wdata := Cat(0.U(24.W), read_data(15, 8)) }
            is("b10".U) { gpr.io.wdata := Cat(0.U(24.W), read_data(23, 16)) }
            is("b11".U) { gpr.io.wdata := Cat(0.U(24.W), read_data(31, 24)) }
          }
        }
      }
    }

  }

}
