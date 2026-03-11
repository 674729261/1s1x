package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class WriteInfo extends Bundle {
  val mem_word = (UInt(32.W))
  val csr_wdata = (UInt(32.W))
  val gpr_wdata = (UInt(32.W))
  val dnpc = (UInt(32.W))
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
  val interruption = (Bool())
}
class MessageEXU2LSU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
  val controls = (new ControlSignalsEXU)
  val write_info = (new WriteInfo)
  val rd_valid = (Bool())
  val itype = new InstType
}

class ConflictInfoRD extends Bundle {
  val rd_valid = Output(Bool())
  val csr_dest_valid = Output(Bool())
  val interruption = Output(Bool())
  val rd_id = Output(UInt(5.W))
  val csr_id = Output(UInt(12.W))

  val ok_to_forward_rd = Output(Bool())
  val rd_data = Output(UInt(32.W))
}

class EXU() extends Module {
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
    val ifu_flush_valid = Output(Bool())

    val idu_flush_valid = Output(Bool())
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
  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  alu.io.A := in.bits.sources.alu_a_or_mepc_or_mtvec
  alu.io.B := in.bits.sources.alu_b
  alu.io.controls := in.bits.controls.alu_controls

  out.bits.write_info.alu_out := alu.io.out

  branch.io.A := in.bits.sources.src1
  branch.io.B := in.bits.sources.src2
  branch.io.funct3 := in.bits.controls.bra_funct3

  val snpc = in.bits.pc + 4.U(32.W)

  out.bits.write_info.gpr_wdata := Mux1H(
    Seq(
      // in.bits.controls.is_gpr_wdata_from_ram -> fetch_port_in.bits.mem_rdata,
      in.bits.controls.is_gpr_wdata_from_snpc -> snpc,
      in.bits.controls.is_gpr_wdata_from_imm -> in.bits.sources.imm,
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

  val should_branch = in.bits.controls.is_branch && branch.io.jump
  val normal_jump = should_branch || in.bits.controls.is_dnpc_jal_or_jalr
  val should_flush =
    in.bits.controls.is_dnpc_csr_jump || (normal_jump ^ in.bits.predicted_jump)

  out_pc.dnpc := MuxCase(
    snpc,
    Seq(
      normal_jump -> alu.io.out,
      in.bits.controls.is_dnpc_csr_jump -> in.bits.sources.alu_a_or_mepc_or_mtvec
    )
  )

  out.bits.controls.is_ebreak := in.bits.controls.is_ebreak
  out.bits.controls.interruption := in.bits.controls.interruption
  conf.rd_id := in.bits.controls.rd
  conf.rd_valid := has_signal && in.bits.rd_valid
  conf.csr_dest_valid := has_signal && in.bits.controls.is_csr_visit
  conf.csr_id := in.bits.controls.csrd
  conf.interruption := in.bits.itype.is_ecall
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

  out.bits.write_info.dnpc := out_pc.dnpc
  val flush_high = (should_flush && is_first_cycle)
  out_pc.ifu_flush_valid := flush_high
  out_pc.idu_flush_valid := flush_high

  in.ready := (out.fire || !has_signal)

  block(PerformanceCounterLayer) {
    val performancecounter_icache = Module(new PerformanceCounter_ICache)
    performancecounter_icache.clock := clock
    performancecounter_icache.reset := reset
    performancecounter_icache.icache_hit := out.fire && in.bits.in_cache
  }
}
