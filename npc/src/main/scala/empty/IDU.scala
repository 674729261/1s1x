package empty.empty
import chisel3._
import chisel3.util._

class InstFields extends Bundle {
  val rs1 = (UInt(5.W))
  val rs2 = (UInt(5.W))
  val rd = (UInt(5.W))
  val funct3 = (UInt(3.W))
  val funct7 = (UInt(7.W))
  val imm = (UInt(32.W))
  val csr = (UInt(12.W))
}

class InstType extends Bundle {
  val is_arithmetic_imm = (Bool())
  val is_arithmetic_reg = (Bool())
  val is_store = (Bool())
  val is_load = (Bool())
  val is_branch = (Bool())
  val is_jal = (Bool())
  val is_jalr = (Bool())
  val is_lui = (Bool())
  val is_auipc = (Bool())
  val is_ebreak = (Bool())
  val is_ecall = (Bool())
  val is_mret = (Bool())
  val is_csrop = (Bool())
}

class ImmType extends Bundle {
  val is_I = Bool()
  val is_R = Bool()
  val is_S = Bool()
  val is_B = Bool()
  val is_J = Bool()
  val is_U = Bool()
}

class ControlSignals extends Bundle {
  val is_csr_visit = (Bool())
  val is_alu_a_pc = (Bool())
  val is_alu_b_reg = (Bool())
  val is_alu_sub_sra = (Bool())
  val is_alu_force_add = (Bool())
  val is_gpr_wdata_from_ram = (Bool())
  val is_gpr_wdata_from_snpc = (Bool())
  val is_gpr_wdata_from_alu = (Bool())
  val is_gpr_wdata_from_imm = (Bool())
  val is_gpr_wdata_from_csr = (Bool())

  val is_gpr_wen = (Bool())

  val is_ram_word = (Bool())
  val is_ram_half = (Bool())
  val is_ram_byte = (Bool())
  val is_load_unsigned = (Bool())

  val is_ram_valid = (Bool())
  val is_ram_wen = (Bool())

  val is_csr_masked = (Bool())
}

object decodeInstType {
  def apply(inst: UInt, fields: InstFields): InstType = {
    val ret = Wire(new InstType)

    ret.is_arithmetic_imm := (inst(6, 0) === "b0010011".U(7.W))
    ret.is_arithmetic_reg := (inst(6, 0) === "b0110011".U(7.W))
    ret.is_store := (inst(6, 0) === "b0100011".U(7.W))
    ret.is_load := (inst(6, 0) === "b0000011".U(7.W))
    ret.is_branch := (inst(6, 0) === "b1100011".U(7.W))
    ret.is_jal := (inst(6, 0) === "b1101111".U(7.W))
    ret.is_jalr := (inst(6, 0) === "b1100111".U(7.W))
    ret.is_lui := (inst(6, 0) === "b0110111".U(7.W))
    ret.is_auipc := (inst(6, 0) === "b0010111".U(7.W))
    ret.is_csrop := (inst(6, 0) === "b1110011".U(7.W))
    val is_funct3_zero = (fields.funct3 === "b000".U(3.W))
    ret.is_ebreak := ret.is_csrop && is_funct3_zero && !inst(21) && inst(20)
    ret.is_ecall := ret.is_csrop && is_funct3_zero && !inst(21) && !inst(20)
    ret.is_mret := ret.is_csrop && is_funct3_zero && inst(21)
    return ret
  }
}

object decodeImmType {
  def apply(inst: UInt, it: InstType): ImmType = {
    val ret = Wire(new ImmType)

    ret.is_I := it.is_arithmetic_imm || it.is_ebreak || it.is_load || it.is_jalr
    ret.is_R := it.is_arithmetic_reg
    ret.is_S := it.is_store
    ret.is_B := it.is_branch
    ret.is_J := it.is_jal
    ret.is_U := it.is_auipc || it.is_lui

    return ret
  }
}

object decodeInstFields {
  def apply(inst: UInt, immType: ImmType): InstFields = {
    val ret = Wire(new InstFields)

    ret.rs1 := inst(19, 15)
    ret.rs2 := inst(24, 20)
    ret.rd := inst(11, 7)
    ret.funct3 := inst(14, 12)
    ret.funct7 := inst(31, 25)
    ret.csr := inst(31, 20)

    val imm_B = Wire(UInt(32.W))
    val imm_S = Wire(UInt(32.W))
    val imm_J = Wire(UInt(32.W))
    val imm_U = Wire(UInt(32.W))
    val imm_I = Wire(UInt(32.W))

    imm_B := Cat(
      Fill(20, inst(31)),
      inst(7),
      inst(30, 25),
      inst(11, 8),
      0.U(1.W)
    )
    imm_S := Cat(
      Fill(20, inst(31)),
      inst(31, 25),
      inst(11, 7)
    )
    imm_J := Cat(
      Fill(12, inst(31)),
      inst(19, 12),
      inst(20),
      inst(30, 21),
      0.U(1.W)
    )

    imm_U := Cat(
      inst(31, 12),
      0.U(12.W)
    )
    imm_I := Cat(
      Fill(20, inst(31)),
      inst(31, 20)
    )

    ret.imm := Mux1H(
      Seq(
        immType.is_B -> imm_B,
        immType.is_S -> imm_S,
        immType.is_J -> imm_J,
        immType.is_U -> imm_U,
        immType.is_I -> imm_I
      )
    )

    return ret
  }
}

object decodeInstControlSignal {
  def apply(
      inst: UInt,
      it: InstType,
      fields: InstFields,
      imm_type: ImmType
  ): ControlSignals = {
    val ret = Wire(new ControlSignals)
    ret.is_alu_a_pc := imm_type.is_B || imm_type.is_J || it.is_auipc
    ret.is_alu_b_reg := imm_type.is_R
    val is_funct3_zero = (fields.funct3 === "b000".U(3.W))
    ret.is_alu_sub_sra :=
      fields.funct7(5) && !(it.is_arithmetic_imm && is_funct3_zero)

    ret.is_csr_visit := it.is_csrop && !is_funct3_zero
    ret.is_alu_force_add := it.is_store || it.is_load || it.is_branch || it.is_auipc || it.is_jal || it.is_jalr
    ret.is_gpr_wdata_from_ram := it.is_load
    ret.is_gpr_wdata_from_snpc := it.is_jal || it.is_jalr
    ret.is_gpr_wdata_from_imm := it.is_lui
    ret.is_gpr_wdata_from_csr := ret.is_csr_visit
    ret.is_gpr_wdata_from_alu := (!ret.is_gpr_wdata_from_csr) && (!ret.is_gpr_wdata_from_imm) && (!ret.is_gpr_wdata_from_ram) && (!ret.is_gpr_wdata_from_snpc)
    ret.is_gpr_wen := ret.is_csr_visit || imm_type.is_U || it.is_load || imm_type.is_R || it.is_arithmetic_imm || it.is_jal || it.is_jalr

    ret.is_ram_byte := (fields.funct3(1, 0) === "b00".U(2.W))
    ret.is_ram_word := (fields.funct3(1, 0) === "b10".U(2.W))
    ret.is_ram_half := (fields.funct3(1, 0) === "b01".U(2.W))
    ret.is_load_unsigned := fields.funct3(2)

    ret.is_ram_valid := it.is_load || it.is_store
    ret.is_ram_wen := it.is_store

    ret.is_csr_masked := fields.funct3(1)
    return ret
  }
}

class Operands extends Bundle {
  val src1 = UInt(32.W)
  val src2 = UInt(32.W)
  val csr = UInt(32.W)
  val mtvec = UInt(32.W)
  val mepc = UInt(32.W)
}

class IDU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val inst = Input(UInt(32.W))
  })

  val out = IO(new Bundle {
    val pc = Output(UInt(32.W))

    val fields = Output(new InstFields)
    val itype = Output(new InstType)
    val controls = Output(new ControlSignals)

    val sources = Output(new Operands)
  })

  val fetch_port_out = IO(new Bundle {
    val gpr_raddr1 = Output(UInt(5.W))
    val gpr_raddr2 = Output(UInt(5.W))

    val csr_raddr = Output(UInt(12.W))
  })

  val fetch_port_in = IO(new Bundle {
    val gpr_rdata1 = Input(UInt(32.W))
    val gpr_rdata2 = Input(UInt(32.W))
    val csr_rdata = Input(UInt(32.W))

    val csr_mtvec = Input(UInt(32.W))
    val csr_mepc = Input(UInt(32.W))
  })

  out.pc := in.pc

  val imm_type = Wire(new ImmType)
  val fields = Wire(new InstFields)
  val inst_type = Wire(new InstType)
  val control_signals = Wire(new ControlSignals)
  out.fields := fields
  out.itype := inst_type
  out.controls := control_signals

  imm_type := decodeImmType(in.inst, inst_type)
  fields := decodeInstFields(in.inst, imm_type)
  inst_type := decodeInstType(in.inst, fields)
  control_signals := decodeInstControlSignal(
    in.inst,
    inst_type,
    fields,
    imm_type
  )

  fetch_port_out.csr_raddr := fields.csr
  fetch_port_out.gpr_raddr1 := fields.rs1
  fetch_port_out.gpr_raddr2 := fields.rs2

  out.sources.csr := fetch_port_in.csr_rdata
  out.sources.src1 := fetch_port_in.gpr_rdata1
  out.sources.src2 := fetch_port_in.gpr_rdata2
  out.sources.mtvec := fetch_port_in.csr_mtvec
  out.sources.mepc := fetch_port_in.csr_mepc
}
