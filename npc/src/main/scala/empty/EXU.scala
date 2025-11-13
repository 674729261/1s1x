package empty.empty
import chisel3._
import chisel3.util._
import empty.ALU

class WriteInfo extends Bundle {
  val gpr_waddr = (UInt(5.W))
  val gpr_wdata = (UInt(32.W))
  val gpr_wen = (Bool())
  val mem_waddr = (UInt(32.W))
  val mem_word = (UInt(32.W))
  val mem_lower2bit = (UInt(2.W))
  val mem_wen = (Bool())
  val csr_addr = (UInt(12.W))
  val csr_wdata = (UInt(32.W))
  val csr_mtvec = (UInt(32.W))
  val csr_mepc = (UInt(32.W))
  val csr_wen = (UInt(32.W))
  val is_rdata_from_csr = (Bool())

  val should_branch = (Bool())
  val alu_out = (UInt(32.W))

}

class EXU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val inst = Input(UInt(32.W))

    val fields = Input(new InstFields)
    val itype = Input(new InstType)
    val controls = Input(new ControlSignals)

  })

  val out = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val controls = Output(new ControlSignals)
    val itype = Output(new InstType)

    val write_info = Output(new WriteInfo)
  })

  val fetch_port_out = IO(new Bundle {
    val gpr_raddr1 = Output(UInt(5.W))
    val gpr_raddr2 = Output(UInt(5.W))

    val csr_raddr = Output(UInt(12.W))

    val mem_raddr = Output(UInt(32.W))
    val mem_rlower2bit = Output(UInt(32.W))
    val mem_rvalid = Output(Bool())
    val is_word = Output(Bool())
    val is_half = Output(Bool())
    val is_byte = Output(Bool())
    val is_unsigned = Output(Bool())
  })

  val fetch_port_in = IO(new Bundle {
    val gpr_rdata1 = Input(UInt(32.W))
    val gpr_rdata2 = Input(UInt(32.W))
    val csr_rdata = Input(UInt(32.W))
    val mem_rdata = Input(UInt(32.W))
    val csr_mtvec = Input(UInt(32.W))
    val csr_mepc = Input(UInt(32.W))
  })

  out.pc := in.pc
  out.controls := in.controls
  out.itype := in.itype

  fetch_port_out.is_byte := in.controls.is_ram_byte
  fetch_port_out.is_half := in.controls.is_ram_half
  fetch_port_out.is_word := in.controls.is_ram_word
  fetch_port_out.is_unsigned := in.controls.is_load_unsigned

  val alu = Module(new ALU(32))
  val branch = Module(new Branch(32))

  fetch_port_out.gpr_raddr1 := in.fields.rs1
  fetch_port_out.gpr_raddr2 := in.fields.rs2

  fetch_port_out.mem_raddr := Cat(alu.io.out(31, 2), "b00".U(2.W))
  fetch_port_out.mem_rvalid := in.controls.is_ram_valid
  fetch_port_out.mem_rlower2bit := alu.io.out(1, 0)

  fetch_port_out.csr_raddr := in.fields.csr

  alu.io.A := Mux(in.controls.is_alu_a_pc, in.pc, fetch_port_in.gpr_rdata1)
  alu.io.B := Mux(
    in.controls.is_alu_b_reg,
    fetch_port_in.gpr_rdata2,
    in.fields.imm
  )
  alu.io.funct3 := in.fields.funct3
  alu.io.is_sub_sra := in.controls.is_alu_sub_sra
  alu.io.is_force_add := in.controls.is_alu_force_add

  out.write_info.alu_out := alu.io.out

  branch.io.A := fetch_port_in.gpr_rdata1
  branch.io.B := fetch_port_in.gpr_rdata2
  branch.io.funct3 := in.fields.funct3

  out.write_info.gpr_waddr := in.fields.rd

  val snpc = in.pc + 4.U(32.W)

  out.write_info.gpr_wen := in.controls.is_gpr_wen
  out.write_info.gpr_wdata := Mux1H(
    Seq(
      in.controls.is_gpr_wdata_from_ram -> fetch_port_in.mem_rdata,
      in.controls.is_gpr_wdata_from_snpc -> snpc,
      in.controls.is_gpr_wdata_from_imm -> in.fields.imm,
      in.controls.is_gpr_wdata_from_alu -> alu.io.out,
      in.controls.is_gpr_wdata_from_csr -> fetch_port_in.csr_rdata
    )
  )
  out.write_info.is_rdata_from_csr := in.controls.is_gpr_wdata_from_csr

  out.write_info.mem_waddr := alu.io.out
  out.write_info.mem_word := fetch_port_in.gpr_rdata2
  out.write_info.mem_lower2bit := alu.io.out(1, 0)
  out.write_info.mem_wen := in.controls.is_ram_wen
  out.write_info.csr_wdata := Mux(
    in.controls.is_csr_masked,
    fetch_port_in.gpr_rdata1 | fetch_port_in.csr_rdata,
    fetch_port_in.gpr_rdata1
  )
  out.write_info.csr_wen := in.controls.is_csr_visit
  out.write_info.csr_addr := in.fields.csr
  out.write_info.csr_mtvec := fetch_port_in.csr_mtvec
  out.write_info.csr_mepc := fetch_port_in.csr_mepc
  out.write_info.should_branch := in.itype.is_branch && branch.io.jump

}
