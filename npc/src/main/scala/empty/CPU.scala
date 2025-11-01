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

// class Memory extends BlackBox {
//   val io = IO(new Bundle {
//     val clk = Input(Bool())
//     val valid = Input(Bool())
//     val wen = Input(Bool())
//     val waddr = Input(UInt(32.W))
//     val raddr = Input(UInt(32.W))
//     val wmask = Input(UInt(4.W))
//     val wdata = Input(UInt(32.W))
//     val rdata = Output(UInt(32.W))
//   })
// }

// class Trap extends BlackBox {
//   val io = IO(new Bundle {
//     val clk = Input(Bool())
//     val ebreak = Input(Bool())
//   })
// }

class CPU(init_pc: UInt) extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val instr = Input(UInt(32.W))
    val pc = Output(UInt(32.W))

    val ebreak = Output(Bool())
    val valid = Output(Bool())
    val raddr = Output(UInt(32.W))
    val wen = Output(Bool())
    val waddr = Output(UInt(32.W))
    val wdata = Output(UInt(32.W))
    val wmask = Output(UInt(4.W))

    val rdata = Input(UInt(32.W))

    val test_ = Output(UInt(32.W))
  })

  def signExt32(in: UInt, bits: Int): UInt = {
    Cat(Fill(32 - bits, in(bits - 1)), in)
  }

  // val Trapper = Module(new Trap)
  val static_pc_next = Wire(UInt(32.W))
  val dynamic_pc_next = Wire(UInt(32.W))
  val pc = RegNext(next = Cat(dynamic_pc_next(31, 1), 0.U(1.W)), init = init_pc)

  io.pc := pc
  static_pc_next := pc + 4.U(32.W)

  val instDecoder = Module(new DecodeInstr)
  val gpr = Module(new GPR)

  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  // val memory_proxy = Module(new Memory)
  val ramWriter = Module(new RamWriteData)
  val ramLoader = Module(new RamLoadData)

  val csrBank = Module(new CSR)
  csrBank.io.csr := instDecoder.io.csr
  csrBank.io.wen := instDecoder.io.is_csr_visit
  csrBank.io.cur_pc := pc
  csrBank.io.new_cause := 11.U(32.W)
  csrBank.io.interruption := instDecoder.io.is_ecall
  val csr_raw_datasrc = Wire(UInt(32.W))
  csr_raw_datasrc := gpr.io.rdata1
  csrBank.io.wdata := Mux(
    instDecoder.io.is_csr_masked,
    csr_raw_datasrc | csrBank.io.rdata,
    csr_raw_datasrc
  )

  // Trapper.io.clk := clock.asBool
  // Trapper.io.ebreak := instDecoder.io.is_ebreak
  io.ebreak := instDecoder.io.is_ebreak

  val should_branch = Wire(Bool())
  should_branch := instDecoder.io.is_branch && branch.io.jump

  io.test_ := Cat(0.U(31.W), alu.io.out)

  dynamic_pc_next := MuxCase(
    static_pc_next,
    Seq(
      should_branch -> alu.io.out,
      instDecoder.io.is_jal -> alu.io.out,
      instDecoder.io.is_jalr -> alu.io.out,
      instDecoder.io.is_ecall -> csrBank.io.mtvec,
      instDecoder.io.is_mret -> csrBank.io.mepc
    )
  )
  // Mux(
  //   (instDecoder.io.is_branch && branch.io.jump) || instDecoder.io.is_jal || instDecoder.io.is_jalr,
  //   alu.io.out,
  //   static_pc_next
  // )

  branch.io.A := gpr.io.rdata1
  branch.io.B := gpr.io.rdata2
  branch.io.funct3 := instDecoder.io.funct3

  instDecoder.io.inst := io.instr

  alu.io.A := Mux(instDecoder.io.is_alu_a_pc, pc, gpr.io.rdata1)
  alu.io.B := Mux(
    instDecoder.io.is_alu_b_reg,
    gpr.io.rdata2,
    instDecoder.io.imm
  )
  alu.io.funct3 := instDecoder.io.funct3
  alu.io.is_sub_sra := instDecoder.io.is_alu_sub_sra
  alu.io.is_force_add := instDecoder.io.is_alu_force_add

  gpr.io.waddr := instDecoder.io.rd
  gpr.io.wen := instDecoder.io.is_gpr_wen
  gpr.io.raddr1 := instDecoder.io.rs1
  gpr.io.raddr2 := instDecoder.io.rs2
  gpr.io.wdata := Mux1H(
    Seq(
      instDecoder.io.is_gpr_wdata_from_ram -> ramLoader.io.out,
      instDecoder.io.is_gpr_wdata_from_snpc -> static_pc_next,
      instDecoder.io.is_gpr_wdata_from_imm -> instDecoder.io.imm,
      instDecoder.io.is_gpr_wdata_from_alu -> alu.io.out,
      instDecoder.io.is_gpr_wdata_from_csr -> csrBank.io.rdata
    )
  )

  // memory_proxy.io.clk := clock.asBool
  // memory_proxy.io.raddr := Cat(alu.io.out(31, 2), "b00".U(2.W))
  // memory_proxy.io.valid := instDecoder.io.is_ram_valid
  // memory_proxy.io.wen := instDecoder.io.is_ram_wen
  // memory_proxy.io.waddr := alu.io.out
  // memory_proxy.io.wdata := ramWriter.io.out
  // memory_proxy.io.wmask := ramWriter.io.mask

  io.raddr := Cat(alu.io.out(31, 2), "b00".U(2.W))
  io.valid := instDecoder.io.is_ram_valid
  io.wen := instDecoder.io.is_ram_wen
  io.waddr := alu.io.out
  io.wdata := ramWriter.io.out
  io.wmask := ramWriter.io.mask

  ramWriter.io.word := gpr.io.rdata2
  ramWriter.io.is_word := instDecoder.io.is_ram_word
  ramWriter.io.is_half := instDecoder.io.is_ram_half
  ramWriter.io.is_byte := instDecoder.io.is_ram_byte
  ramWriter.io.lower2bit := alu.io.out(1, 0)

  ramLoader.io.is_byte := instDecoder.io.is_ram_byte
  ramLoader.io.is_half := instDecoder.io.is_ram_half
  ramLoader.io.is_word := instDecoder.io.is_ram_word
  ramLoader.io.is_unsigned := instDecoder.io.is_load_unsigned
  // ramLoader.io.word := memory_proxy.io.rdata
  ramLoader.io.word := io.rdata

  ramLoader.io.lower2bit := alu.io.out(1, 0)

}
