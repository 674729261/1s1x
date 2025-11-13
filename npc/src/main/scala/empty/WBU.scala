package empty.empty
import chisel3._
import chisel3.util._

class WBU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val controls = Input(new ControlSignals)
    val itype = Input(new InstType)
    val write_info = Input(new WriteInfo)
  })

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

  })

  out.ebreak := in.itype.is_ebreak

  out.csr_waddr := in.write_info.csr_addr
  out.csr_wen := in.write_info.csr_wen
  out.csr_cur_pc := in.pc
  out.csr_mcause := 11.U(32.W)
  out.csr_interruption := in.itype.is_ecall
  out.csr_wdata := in.write_info.csr_wdata

  out.dnpc := MuxCase(
    in.pc + 4.U(32.W),
    Seq(
      in.write_info.should_branch -> in.write_info.alu_out,
      in.itype.is_jal -> in.write_info.alu_out,
      in.itype.is_jalr -> in.write_info.alu_out,
      in.itype.is_ecall -> in.write_info.csr_mtvec,
      in.itype.is_mret -> in.write_info.csr_mepc
    )
  )

  out.gpr_waddr := in.write_info.gpr_waddr
  out.gpr_wdata := in.write_info.gpr_wdata
  out.gpr_wen := in.write_info.gpr_wen

}
