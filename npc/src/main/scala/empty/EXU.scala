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

class EXU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))

    val fields = Input(new InstFields)
    val itype = Input(new InstType)
    val controls = Input(new ControlSignals)
    val sources = Input(new Operands)

  })

  val out = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val controls = Output(new ControlSignals)
    val itype = Output(new InstType)

    val write_info = Output(new WriteInfo)
  })

  out.pc := in.pc
  out.controls := in.controls
  out.itype := in.itype

  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  alu.io.A := Mux(in.controls.is_alu_a_pc, in.pc, in.sources.src1)
  alu.io.B := Mux(
    in.controls.is_alu_b_reg,
    in.sources.src2,
    in.fields.imm
  )
  alu.io.funct3 := in.fields.funct3
  alu.io.is_sub_sra := in.controls.is_alu_sub_sra
  alu.io.is_force_add := in.controls.is_alu_force_add

  out.write_info.alu_out := alu.io.out

  branch.io.A := in.sources.src1
  branch.io.B := in.sources.src2
  branch.io.funct3 := in.fields.funct3

  out.write_info.gpr_waddr := in.fields.rd

  val snpc = in.pc + 4.U(32.W)

  out.write_info.gpr_wdata := Mux1H(
    Seq(
      // in.controls.is_gpr_wdata_from_ram -> fetch_port_in.mem_rdata,
      in.controls.is_gpr_wdata_from_snpc -> snpc,
      in.controls.is_gpr_wdata_from_imm -> in.fields.imm,
      in.controls.is_gpr_wdata_from_alu -> alu.io.out,
      in.controls.is_gpr_wdata_from_csr -> in.sources.csr
    )
  )

  out.write_info.mem_word := in.sources.src2
  out.write_info.csr_wdata := Mux(
    in.controls.is_csr_masked,
    in.sources.src1 | in.sources.csr,
    in.sources.src1
  )
  out.write_info.csr_addr := in.fields.csr

  val should_branch = in.itype.is_branch && branch.io.jump

  out.write_info.dnpc := MuxCase(
    in.pc + 4.U(32.W),
    Seq(
      should_branch -> alu.io.out,
      in.itype.is_jal -> alu.io.out,
      in.itype.is_jalr -> alu.io.out,
      in.itype.is_ecall -> in.sources.mtvec,
      in.itype.is_mret -> in.sources.mepc
    )
  )

}
