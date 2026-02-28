package empty
import chisel3._
import chisel3.util._

class WriteInfo extends Bundle {
  val gpr_waddr = (UInt(5.W))
  val gpr_wdata = (UInt(32.W))
  val mem_word = (UInt(32.W))
  val csr_addr = (UInt(12.W))
  val csr_wdata = (UInt(32.W))

  val dnpc = UInt(32.W)

  val alu_out = (UInt(32.W))

}

class MessageEXU2LSU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
  val controls = (new ControlSignals)
  val itype = (new InstType)

  val write_info = (new WriteInfo)
}

class EXU() extends Module {
  val in = IO(Flipped(DecoupledIO(new MessageIDU2EXU)))

  val out = IO(DecoupledIO(new MessageEXU2LSU))

  val is_signal_in_latched = RegInit(Bool(), false.B)

  val cpu_out_fire = out.ready && out.valid
  val should_signal_in_latch = in.valid && !is_signal_in_latched
  val signals_in_r = RegEnable(in.bits, should_signal_in_latch)
  is_signal_in_latched := MuxCase(
    is_signal_in_latched,
    Seq(
      should_signal_in_latch -> true.B,
      cpu_out_fire -> false.B
    )
  )

  out.bits.pc := signals_in_r.pc
  out.bits.inst := signals_in_r.inst
  out.bits.controls := signals_in_r.controls
  out.bits.itype := signals_in_r.itype

  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  alu.io.A := Mux(
    signals_in_r.controls.is_alu_a_pc,
    signals_in_r.pc,
    signals_in_r.sources.src1
  )
  alu.io.B := Mux(
    signals_in_r.controls.is_alu_b_reg,
    signals_in_r.sources.src2,
    signals_in_r.fields.imm
  )
  alu.io.funct3 := signals_in_r.fields.funct3
  alu.io.is_sub_sra := signals_in_r.controls.is_alu_sub_sra
  alu.io.is_force_add := signals_in_r.controls.is_alu_force_add

  out.bits.write_info.alu_out := alu.io.out

  branch.io.A := signals_in_r.sources.src1
  branch.io.B := signals_in_r.sources.src2
  branch.io.funct3 := signals_in_r.fields.funct3

  out.bits.write_info.gpr_waddr := signals_in_r.fields.rd

  val snpc = signals_in_r.pc + 4.U(32.W)

  out.bits.write_info.gpr_wdata := Mux1H(
    Seq(
      // signals_in_r.controls.is_gpr_wdata_from_ram -> fetch_port_signals_in_r.mem_rdata,
      signals_in_r.controls.is_gpr_wdata_from_snpc -> snpc,
      signals_in_r.controls.is_gpr_wdata_from_imm -> signals_in_r.fields.imm,
      signals_in_r.controls.is_gpr_wdata_from_alu -> alu.io.out,
      signals_in_r.controls.is_gpr_wdata_from_csr -> signals_in_r.sources.csr
    )
  )

  out.bits.write_info.mem_word := signals_in_r.sources.src2
  out.bits.write_info.csr_wdata := Mux(
    signals_in_r.controls.is_csr_masked,
    signals_in_r.sources.src1 | signals_in_r.sources.csr,
    signals_in_r.sources.src1
  )
  out.bits.write_info.csr_addr := signals_in_r.fields.csr

  val should_branch = signals_in_r.itype.is_branch && branch.io.jump

  out.bits.write_info.dnpc := MuxCase(
    signals_in_r.pc + 4.U(32.W),
    Seq(
      should_branch -> alu.io.out,
      signals_in_r.itype.is_jal -> alu.io.out,
      signals_in_r.itype.is_jalr -> alu.io.out,
      signals_in_r.itype.is_ecall -> signals_in_r.sources.mtvec,
      signals_in_r.itype.is_mret -> signals_in_r.sources.mepc
    )
  )
  out.valid := is_signal_in_latched
  in.ready := out.ready
}
