package empty.empty
import chisel3._
import chisel3.util._
import empty.ALU

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
  val controls = (new ControlSignals)
  val itype = (new InstType)

  val write_info = (new WriteInfo)
}

class EXU() extends Module with RequireAsyncReset {
  val in = IO(Flipped(DecoupledIO(new MessageIDU2EXU)))

  val out = IO(DecoupledIO(new MessageEXU2LSU))

  out.bits.pc := in.bits.pc
  out.bits.controls := in.bits.controls
  out.bits.itype := in.bits.itype

  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  alu.io.A := Mux(
    in.bits.controls.is_alu_a_pc,
    in.bits.pc,
    in.bits.sources.src1
  )
  alu.io.B := Mux(
    in.bits.controls.is_alu_b_reg,
    in.bits.sources.src2,
    in.bits.fields.imm
  )
  alu.io.funct3 := in.bits.fields.funct3
  alu.io.is_sub_sra := in.bits.controls.is_alu_sub_sra
  alu.io.is_force_add := in.bits.controls.is_alu_force_add

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

  val should_branch = in.bits.itype.is_branch && branch.io.jump

  out.bits.write_info.dnpc := MuxCase(
    in.bits.pc + 4.U(32.W),
    Seq(
      should_branch -> alu.io.out,
      in.bits.itype.is_jal -> alu.io.out,
      in.bits.itype.is_jalr -> alu.io.out,
      in.bits.itype.is_ecall -> in.bits.sources.mtvec,
      in.bits.itype.is_mret -> in.bits.sources.mepc
    )
  )
  out.valid := in.valid
  in.ready := in.valid
}
