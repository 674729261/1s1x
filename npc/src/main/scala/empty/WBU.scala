package empty.empty
import chisel3._
import chisel3.util._

class WBU() extends Module with RequireAsyncReset {
  val in = IO(Flipped(DecoupledIO(new MessageLSU2WBU)))

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

  })

  out.ebreak := in.bits.itype.is_ebreak && in.valid

  out.csr_waddr := in.bits.write_info.csr_addr
  out.csr_wen := in.bits.controls.is_csr_visit && in.valid
  out.csr_cur_pc := in.bits.pc
  out.csr_mcause := 11.U(32.W)
  out.csr_interruption := in.bits.itype.is_ecall
  out.csr_wdata := in.bits.write_info.csr_wdata

  out.dnpc := Mux(in.valid, in.bits.write_info.dnpc, in.bits.pc)

  out.gpr_waddr := in.bits.write_info.gpr_waddr
  out.gpr_wdata := in.bits.write_info.gpr_wdata
  out.gpr_wen := in.bits.controls.is_gpr_wen && in.valid

  in.ready := in.valid
  out.ok_to_step := in.valid
}
