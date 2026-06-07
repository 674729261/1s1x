package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class WriteInfo extends Bundle {
  val mem_word_or_csr_wdata = (UInt(32.W))
  val gpr_wdata = (UInt(32.W))
  val dnpc = (UInt(32.W))
  val mtvec = (UInt(32.W))
  val alu_out = (UInt(32.W))
}

class PerformanceCounter_ICache extends ExtModule {
  val clock = IO(Input(Clock()))
  val reset = IO(Input(Reset()))
  val icache_hit = IO(Input(Bool()))

}

class ControlSignalsEXU extends Bundle {
  val is_csr_visit = (Bool())

  val is_gpr_wen = (Bool())
  val is_gpr_wdata_from_ram = (Bool())

  val is_ram_word = (Bool())
  val is_ram_half = (Bool())
  val is_ram_byte = (Bool())
  val is_load_unsigned = (Bool())

  val is_ram_valid = (Bool())
  val is_ram_wen = (Bool())

  val rd = UInt(5.W)
  val csrd = UInt(12.W)
  val is_ebreak = (Bool())
}
class MessageEXU2LSU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
  val controls = (new ControlSignalsEXU)
  val write_info = (new WriteInfo)
  val rd_valid = (Bool())
  val itype = new InstType

  val exeption = (Bool())
  val cause = (UInt(4.W))
  val csr_jump = (Bool())
}

class ConflictInfoRD extends Bundle {
  val rd_valid = Output(Bool())
  val csr_dest_valid = Output(Bool())
  val rd_id = Output(UInt(5.W))
  val csr_id = Output(UInt(12.W))

  val ok_to_forward_rd = Output(Bool())
  val rd_data = Output(UInt(32.W))
}

class EXU() extends PrefixedModule {
  val in = IO(Flipped(DecoupledIO(new MessageIDU2EXU)))

  val out = IO(DecoupledIO(new MessageEXU2LSU))
  val conf = IO(new ConflictInfoRD)
  val btb = IO(new Bundle {
    val write_pc = Output(UInt(32.W))
    val target = Output(UInt(32.W))
    val is_jump_taken = Output(Bool())
    val init_cnt = Output(UInt(2.W))
    val wen = Output(Bool())
  })
  val out_pc = IO(new Bundle {
    val dnpc = Output(UInt(32.W))
    val flush_valid = Output(Bool())
  })
  val flush = IO(new Bundle {
    val valid = Input(Bool())
  })

  val has_signal_r = RegInit(Bool(), false.B)
  has_signal_r := MuxCase(
    has_signal_r,
    Seq(
      (in.fire && !flush.valid) -> true.B,
      (out.fire || flush.valid) -> false.B
    )
  )
  val has_signal = has_signal_r && !flush.valid
  out.bits.pc := in.bits.pc
  out.bits.inst := in.bits.inst
  out.bits.controls := in.bits.controls
  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  val imm_I = Cat(Fill(20, in.bits.inst(31)), in.bits.inst(31, 20))
  val imm_S = Cat(Fill(20, in.bits.inst(31)), in.bits.inst(31, 25), in.bits.inst(11, 7))
  val imm_B = Cat(Fill(20, in.bits.inst(31)), in.bits.inst(7), in.bits.inst(30, 25), in.bits.inst(11, 8), 0.U(1.W))
  val imm_U = Cat(in.bits.inst(31, 12), 0.U(12.W))
  val imm_J = Cat(Fill(12, in.bits.inst(31)), in.bits.inst(19, 12), in.bits.inst(20), in.bits.inst(30, 21), 0.U(1.W))

  val alu_imm = Mux1H(
    Seq(
      in.bits.itype.is_branch -> imm_B,
      in.bits.itype.is_store -> imm_S,
      in.bits.itype.is_jal -> imm_J,
      (in.bits.itype.is_auipc || in.bits.itype.is_lui) -> imm_U,
      (in.bits.itype.is_arithmetic_imm || in.bits.itype.is_load || in.bits.itype.is_jalr || in.bits.itype.is_ebreak) -> imm_I
    )
  )

  val alu_a = MuxCase(
    in.bits.sources.src1,
    Seq(
      (in.bits.itype.is_lui || in.bits.itype.is_mret) -> 0.U(32.W),
      (in.bits.itype.is_branch || in.bits.itype.is_jal || in.bits.itype.is_auipc) -> in.bits.pc
    )
  )
  val alu_b_raw = Mux(
    in.bits.itype.is_mret || in.bits.itype.is_arithmetic_reg,
    in.bits.sources.src2_or_csr,
    alu_imm
  )
  val alu_b = Mux(
    in.bits.controls.alu_controls.is_alu_b_inv,
    ~alu_b_raw,
    alu_b_raw
  )

  alu.io.A := alu_a
  alu.io.B := alu_b
  alu.io.controls := in.bits.controls.alu_controls

  out.bits.write_info.alu_out := alu.io.out

  branch.io.A := in.bits.sources.src1
  branch.io.B := in.bits.sources.src2_or_csr
  branch.io.funct3 := in.bits.controls.bra_funct3

  val snpc = in.bits.pc + 4.U(32.W)

  out.bits.write_info.gpr_wdata := Mux1H(
    Seq(
      in.bits.controls.is_gpr_wdata_from_snpc -> snpc,
      in.bits.controls.is_gpr_wdata_from_alu -> alu.io.out,
      in.bits.controls.is_gpr_wdata_from_csr -> in.bits.sources.src2_or_csr
    )
  )

  out.bits.write_info.mem_word_or_csr_wdata := Mux(
    in.bits.itype.is_store,
    in.bits.sources.src2_or_csr,
    Mux(
      in.bits.controls.is_csr_masked,
      in.bits.sources.src1 | in.bits.sources.src2_or_csr,
      in.bits.sources.src1
    )
  )

  val should_branch = in.bits.controls.is_branch && branch.io.jump
  val static_jump = should_branch || in.bits.itype.is_jal
  val should_flush =
    in.bits.controls.is_dnpc_csr_jump || in.bits.itype.is_jalr || (static_jump ^ in.bits.predicted_jump)

  out_pc.dnpc := MuxCase(
    snpc,
    Seq(
      (should_branch || in.bits.controls.is_dnpc_jal_or_jalr || in.bits.controls.is_dnpc_csr_jump) -> alu.io.out
    )
  )

  out.bits.controls.is_ebreak := in.bits.controls.is_ebreak
  conf.rd_id := in.bits.controls.rd
  conf.rd_valid := has_signal && in.bits.rd_valid
  conf.csr_dest_valid := has_signal && in.bits.controls.is_csr_visit
  conf.csr_id := in.bits.controls.csrd
  conf.ok_to_forward_rd := !in.bits.controls.is_gpr_wdata_from_ram
  conf.rd_data := out.bits.write_info.gpr_wdata

  out.bits.itype := in.bits.itype
  out.bits.rd_valid := in.bits.rd_valid
  out.valid := has_signal
  val is_first_cycle = RegNext(in.fire, false.B)
  btb.wen := (in.bits.controls.is_branch || in.bits.itype.is_jal) && is_first_cycle
  btb.write_pc := in.bits.pc
  btb.target := alu.io.out
  btb.init_cnt := Mux(
    in.bits.inst(31) || in.bits.itype.is_jal,
    2.U(2.W),
    1.U(2.W)
  )
  btb.is_jump_taken := branch.io.jump || in.bits.itype.is_jal

  out.bits.write_info.mtvec := in.bits.sources.mtvec
  out.bits.write_info.dnpc := out_pc.dnpc
  val flush_high = (should_flush && is_first_cycle)
  out_pc.flush_valid := flush_high

  out.bits.exeption := in.bits.exception
  out.bits.cause := in.bits.cause
  out.bits.csr_jump := in.bits.controls.is_dnpc_csr_jump

  in.ready := (out.fire || !has_signal)

  block(PerformanceCounterLayer) {
    val performancecounter_icache = Module(new PerformanceCounter_ICache)
    performancecounter_icache.clock := clock
    performancecounter_icache.reset := reset
    performancecounter_icache.icache_hit := out.fire && in.bits.in_cache
  }
}
