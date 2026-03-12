package empty
import chisel3._
import chisel3.util._

class WBU() extends Module {
  val in = IO(Flipped(DecoupledIO(new MessageLSU2WBU)))
  val conf = IO(new ConflictInfoRD)
  val out = IO(new Bundle {
    val ebreak = Output(Bool())
    val dnpc = Output(UInt(32.W))

    val csr_waddr = Output(UInt(12.W))
    val csr_wen = Output(Bool())
    val csr_wdata = Output(UInt(32.W))
    val csr_cur_pc = Output(UInt(32.W))
    val csr_interruption = Output(UInt(32.W))
    val csr_mcause = Output(UInt(32.W))

    val gpr_waddr = Output(UInt(5.W))
    val gpr_wdata = Output(UInt(32.W))
    val gpr_wen = Output(Bool())

    val ok_to_step = Output(Bool())
    val retire_pc = Output(UInt(32.W))
    val retire_inst = Output(UInt(32.W))

    val inst_type = Output(new InstType)

  })
  val has_signal = RegInit(Bool(), false.B)
  has_signal := Mux(in.fire, true.B, false.B)

  out.ebreak := in.bits.controls.is_ebreak && has_signal

  out.csr_waddr := in.bits.controls.csrd
  out.csr_wen := in.bits.controls.is_csr_visit && has_signal && !in.bits.exception
  out.csr_cur_pc := in.bits.pc
  out.csr_mcause := in.bits.cause
  out.csr_interruption := in.bits.exception && has_signal
  out.csr_wdata := in.bits.write_info.csr_wdata_or_mtvec

  out.dnpc := in.bits.write_info.dnpc

  out.gpr_waddr := in.bits.controls.rd
  out.gpr_wdata := in.bits.write_info.gpr_wdata
  out.gpr_wen := in.bits.controls.is_gpr_wen && has_signal && !in.bits.exception
  conf.rd_id := in.bits.controls.rd
  conf.rd_valid := has_signal && in.bits.rd_valid
  conf.csr_dest_valid := has_signal && in.bits.itype.is_csrop
  conf.csr_id := in.bits.controls.csrd
  conf.rd_valid := in.bits.rd_valid
  conf.ok_to_forward_rd := true.B
  conf.rd_data := in.bits.write_info.gpr_wdata
  in.ready := in.valid
  out.ok_to_step := has_signal
  out.retire_pc := in.bits.pc
  out.retire_inst := in.bits.inst
  out.inst_type := in.bits.itype
}
