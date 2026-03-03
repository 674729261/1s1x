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

    val clear_icache_valid = Output(Bool())
    val clear_icache_ok = Input(Bool())

    val ok_to_step = Output(Bool())
    val retire_pc = Output(UInt(32.W))
    val retire_inst = Output(UInt(32.W))

    val inst_type = Output(new InstType)

  })
  val has_signal = RegInit(Bool(), false.B)
  has_signal := Mux(in.fire, true.B, false.B)
  out.clear_icache_valid := in.bits.itype.is_fence && in.valid
  val fence_fire = out.clear_icache_valid && out.clear_icache_ok

  out.ebreak := in.bits.itype.is_ebreak && in.valid

  out.csr_waddr := in.bits.write_info.csr_addr
  out.csr_wen := in.bits.controls.is_csr_visit && in.valid
  out.csr_cur_pc := in.bits.pc
  out.csr_mcause := 11.U(32.W)
  out.csr_interruption := in.bits.itype.is_ecall && in.valid
  out.csr_wdata := in.bits.write_info.csr_wdata

  out.dnpc := Mux(in.valid, in.bits.write_info.dnpc, in.bits.pc)

  out.gpr_waddr := in.bits.write_info.gpr_waddr
  out.gpr_wdata := in.bits.write_info.gpr_wdata
  out.gpr_wen := in.bits.controls.is_gpr_wen && in.valid
  conf.rd_id := in.bits.write_info.gpr_waddr
  conf.rd_valid := has_signal && in.bits.rd_valid
  in.ready := in.valid && (fence_fire || !in.bits.itype.is_fence)
  out.ok_to_step := has_signal
  out.retire_pc := in.bits.pc
  out.retire_inst := in.bits.inst
  out.inst_type := in.bits.itype
}
