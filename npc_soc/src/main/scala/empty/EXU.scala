package empty
import chisel3._
import chisel3.util._

class WriteInfo extends Bundle {
  val gpr_waddr = (UInt(5.W))
  val gpr_wdata = (UInt(32.W))
  val mem_word = (UInt(32.W))
  val csr_addr = (UInt(12.W))
  val csr_wdata = (UInt(32.W))
  val dnpc = (UInt(32.W))
  val alu_out = (UInt(32.W))

}

class MessageEXU2LSU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
  val controls = (new ControlSignals)
  val itype = (new InstType)
  val write_info = (new WriteInfo)
  val rd_valid = (Bool())
}

class ConflictInfoRD extends Bundle {
  val rd_valid = Output(Bool())
  val rd_id = Output(UInt(5.W))
}

class EXU() extends Module {
  val in = IO(Flipped(DecoupledIO(new MessageIDU2EXU)))

  val out = IO(DecoupledIO(new MessageEXU2LSU))
  val conf = IO(new ConflictInfoRD)
  val out_pc = IO(new Bundle {
    val dnpc = Output(UInt(32.W))
    val valid = Output(Bool())
    val ready = Input(Bool())

    val idu_flush_valid = Output(Bool())
    val idu_flush_ready = Input(Bool())
  })

  val has_signal = RegInit(Bool(), false.B)
  has_signal := MuxCase(
    has_signal,
    Seq(
      in.fire -> true.B,
      out.fire -> false.B
    )
  )
  out.bits.pc := in.bits.pc
  out.bits.inst := in.bits.inst
  out.bits.controls := in.bits.controls
  out.bits.itype := in.bits.itype

  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  alu.io.A := in.bits.sources.alu_a
  alu.io.B := in.bits.sources.alu_b
  alu.io.controls := in.bits.controls.alu_controls

  out.bits.write_info.alu_out := alu.io.out

  branch.io.A := in.bits.sources.src1
  branch.io.B := in.bits.sources.src2
  branch.io.funct3 := in.bits.fields.funct3

  out.bits.write_info.gpr_waddr := in.bits.fields.rd

  val snpc = in.bits.pc + 4.U(32.W)

  out.bits.write_info.gpr_wdata := Mux1H(
    Seq(
      // in.bits.controls.is_gpr_wdata_from_ram -> fetch_port_in.bits.mem_rdata,
      in.bits.controls.is_gpr_wdata_from_snpc -> snpc,
      in.bits.controls.is_gpr_wdata_from_imm -> in.bits.fields.imm,
      in.bits.controls.is_gpr_wdata_from_alu -> alu.io.out,
      in.bits.controls.is_gpr_wdata_from_csr -> in.bits.sources.csr
    )
  )

  out.bits.write_info.mem_word := in.bits.sources.src2
  out.bits.write_info.csr_wdata := Mux(
    in.bits.controls.is_csr_masked,
    in.bits.sources.src1 | in.bits.sources.csr,
    in.bits.sources.src1
  )
  out.bits.write_info.csr_addr := in.bits.fields.csr
  val should_flush = (snpc === out_pc.dnpc)
  val should_branch = in.bits.itype.is_branch && branch.io.jump

  out_pc.dnpc := MuxCase(
    snpc,
    Seq(
      should_branch -> alu.io.out,
      in.bits.itype.is_jal -> alu.io.out,
      in.bits.itype.is_jalr -> alu.io.out,
      in.bits.itype.is_ecall -> in.bits.sources.mtvec,
      in.bits.itype.is_mret -> in.bits.sources.mepc
    )
  )
  out.bits.write_info.dnpc := out_pc.dnpc
  out_pc.valid := (out.valid && should_flush)
  out_pc.idu_flush_valid := (out.valid && should_flush)

  conf.rd_id := in.bits.fields.rd
  conf.rd_valid := has_signal && in.bits.rd_valid
  out.bits.rd_valid := in.bits.rd_valid
  out.valid := has_signal

  val ifu_dnpc_fire = out_pc.ready && out_pc.valid
  val pending_dnpc = RegInit(Bool(), false.B)

  pending_dnpc := MuxCase(
    pending_dnpc,
    Seq(
      (in.fire && should_flush) -> true.B,
      ifu_dnpc_fire -> false.B
    )
  )

  val idu_flush_fire = out_pc.idu_flush_ready && out_pc.idu_flush_valid
  val pending_idu_flush = RegInit(Bool(), false.B)
  pending_idu_flush := MuxCase(
    pending_idu_flush,
    Seq(
      (in.fire && should_flush) -> true.B,
      idu_flush_fire -> false.B
    )
  )

  in.ready := (out.fire || !has_signal) && !pending_dnpc && !pending_idu_flush
}
