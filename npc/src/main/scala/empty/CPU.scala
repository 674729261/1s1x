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
import _root_.empty.empty.Branch

class Memory extends BlackBox {
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

class Trap extends BlackBox {
  val io = IO(new Bundle {
    val clk = Input(Bool())
    val ebreak = Input(Bool())
  })
}

class CPU(init_pc: UInt) extends Module with RequireAsyncReset {
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

  dynamic_pc_next := static_pc_next

  val gpr = Module(new GPR)
  val instDecoder = Module(new DecodeInstr)
  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  branch.io.A := gpr.io.rdata1
  branch.io.B := gpr.io.rdata2
  branch.io.funct3 := instDecoder.io.funct3

  instDecoder.io.inst := io.instr

  // default for arithmetic_reg inst

  alu.io.A := Mux(instDecoder.io.is_alu_a_pc, pc, gpr.io.rdata1)
  alu.io.B := Mux(
    instDecoder.io.is_alu_b_imm,
    instDecoder.io.imm,
    gpr.io.rdata2
  )
  alu.io.funct3 := instDecoder.io.funct3
  alu.io.is_sub_sra := instDecoder.io.funct7(5)
  alu.io.is_force_add := instDecoder.io.is_alu_force_add

  gpr.io.waddr := instDecoder.io.rd
  gpr.io.wen := instDecoder.io.is_gpr_write
  gpr.io.raddr1 := instDecoder.io.rs1
  gpr.io.raddr2 := instDecoder.io.rs2
  gpr.io.wdata := alu.io.out

  // default for sw

  val mem_wdata = Wire(Vec(4, UInt(8.W)))
  mem_wdata := gpr.io.rdata2.asTypeOf(Vec(4, UInt(8.W)))
  val memory_proxy = Module(new Memory)
  memory_proxy.io.clk := clock.asBool
  memory_proxy.io.raddr := alu.io.out
  memory_proxy.io.valid := false.B
  memory_proxy.io.wen := false.B
  memory_proxy.io.waddr := alu.io.out
  memory_proxy.io.wdata := mem_wdata.asUInt
  memory_proxy.io.wmask := "b1111".U(4.W)

  when(instDecoder.io.is_arithmetic_reg) {}

  // B of ALU

}
