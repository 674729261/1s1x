package empty.empty
import chisel3._
import chisel3.util._

class MessageLSU2WBU extends Bundle {
  val pc = (UInt(32.W))
  val controls = (new ControlSignals)
  val itype = (new InstType)
  val write_info = (new WriteInfo)
}

class LSU() extends Module with RequireAsyncReset {

  val in = IO(Flipped(DecoupledIO(new MessageEXU2LSU)))

  val out = IO(DecoupledIO(new MessageLSU2WBU))

  val fetch_port_out = IO(new Bundle {
    val mem_raddr = Output(UInt(32.W))
    val mem_rlower2bit = Output(UInt(32.W))
    val mem_valid = Output(Bool())
    val is_word = Output(Bool())
    val is_half = Output(Bool())
    val is_byte = Output(Bool())
    val is_unsigned = Output(Bool())

    val mem_waddr = Output(UInt(32.W))
    val mem_lower2bit = Output(UInt(2.W))
    val mem_wdata = Output(UInt(32.W))
    val mem_wen = Output(Bool())
  })

  val fetch_port_in = IO(new Bundle {
    val mem_rdata = Input(UInt(32.W))
  })

  fetch_port_out.is_byte := in.bits.controls.is_ram_byte
  fetch_port_out.is_half := in.bits.controls.is_ram_half
  fetch_port_out.is_word := in.bits.controls.is_ram_word
  fetch_port_out.is_unsigned := in.bits.controls.is_load_unsigned

  fetch_port_out.mem_raddr := Cat(
    in.bits.write_info.alu_out(31, 2),
    "b00".U(2.W)
  )
  fetch_port_out.mem_valid := in.bits.controls.is_ram_valid
  fetch_port_out.mem_rlower2bit := in.bits.write_info.alu_out(1, 0)

  fetch_port_out.mem_wdata := in.bits.write_info.mem_word
  fetch_port_out.is_word := in.bits.controls.is_ram_word
  fetch_port_out.is_half := in.bits.controls.is_ram_half
  fetch_port_out.is_byte := in.bits.controls.is_ram_byte
  fetch_port_out.mem_lower2bit := in.bits.write_info.alu_out(1, 0)
  fetch_port_out.mem_waddr := in.bits.write_info.alu_out
  fetch_port_out.mem_wen := in.bits.controls.is_ram_wen

  out.bits.pc := in.bits.pc
  out.bits.controls := in.bits.controls
  out.bits.itype := in.bits.itype

  out.bits.write_info := in.bits.write_info
  when(in.bits.controls.is_gpr_wdata_from_ram) {
    out.bits.write_info.gpr_wdata := fetch_port_in.mem_rdata
  }
  in.ready := true.B
  out.valid := true.B
}
