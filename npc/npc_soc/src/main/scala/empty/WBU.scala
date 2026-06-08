package empty
import chisel3._
import chisel3.util._

class WBU() extends PrefixedModule {
  val in = IO(Flipped(DecoupledIO(new MessageLSU2WBU)))
  val out = IO(new Bundle {
    val ebreak = Output(Bool())
    val dnpc = Output(UInt(32.W))

    val csr_waddr = Output(UInt(12.W))
    val csr_wen = Output(Bool())
    val csr_wdata = Output(UInt(32.W))
    val csr_cur_pc = Output(UInt(32.W))
    val csr_interruption = Output(Bool())
    val csr_mcause = Output(UInt(32.W))

    val gpr_waddr = Output(UInt(5.W))
    val gpr_wdata = Output(UInt(32.W))
    val gpr_wen = Output(Bool())

    val ok_to_step = Output(Bool())
    val retire_pc = Output(UInt(32.W))
    val retire_inst = Output(UInt(32.W))

    val inst_type = Output(new InstType)

  })

  in.ready := true.B
  val commit = in.valid

  out.ebreak := in.bits.controls.is_ebreak && commit

  out.csr_waddr := in.bits.controls.csrd
  out.csr_wen := in.bits.controls.is_csr_visit && commit && !in.bits.exception
  out.csr_cur_pc := in.bits.pc
  out.csr_mcause := in.bits.cause
  out.csr_interruption := in.bits.exception && commit
  out.csr_wdata := in.bits.write_info.mem_word_or_csr_wdata

  out.dnpc := in.bits.write_info.dnpc

  out.gpr_waddr := in.bits.controls.rd
  out.gpr_wdata := in.bits.write_info.gpr_wdata
  out.gpr_wen := in.bits.controls.is_gpr_wen && commit && !in.bits.exception

  out.ok_to_step := commit
  out.retire_pc := in.bits.pc
  out.retire_inst := in.bits.inst
  out.inst_type := in.bits.itype
}
