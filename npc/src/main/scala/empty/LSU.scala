package empty.empty
import chisel3._
import chisel3.util._
import empty.RamWriteData
import empty.RamLoadData
import empty.MemAccessBus
import svsim.CommonCompilationSettings.Timescale.Unit.s

class MessageLSU2WBU extends Bundle {
  val pc = (UInt(32.W))
  val controls = (new ControlSignals)
  val itype = (new InstType)
  val write_info = (new WriteInfo)
}

class LSU() extends Module with RequireAsyncReset {

  val in = IO(Flipped(DecoupledIO(new MessageEXU2LSU)))

  val out = IO(DecoupledIO(new MessageLSU2WBU))

  val fetch_port = IO(new MemAccessBus)

  val ramWriter = Module(new RamWriteData)
  val ramLoader = Module(new RamLoadData)

  val should_mem_access = Wire(Bool())

  val sIDLE :: sWAIT_RESP :: sWAIT :: Nil = Enum(3)
  val state = RegInit(sIDLE)
  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(should_mem_access && fetch_port.reqReady, sWAIT_RESP, sIDLE),
      sWAIT_RESP -> Mux(fetch_port.respValid, sWAIT, sWAIT_RESP),
      sWAIT -> Mux(out.ready, sIDLE, sWAIT)
    )
  )

  val rdata_reg =
    RegEnable(fetch_port.rdata, state === sWAIT_RESP && fetch_port.respValid)

  fetch_port.respReady := state === sWAIT_RESP

  ramLoader.io.word := rdata_reg
  ramLoader.io.is_byte := in.bits.controls.is_ram_byte
  ramLoader.io.is_half := in.bits.controls.is_ram_half
  ramLoader.io.is_word := in.bits.controls.is_ram_word
  ramLoader.io.is_unsigned := in.bits.controls.is_load_unsigned
  // ramLoader.io.word := memory_proxy.io.rdata
  ramLoader.io.lower2bit := in.bits.write_info.alu_out(1, 0)

  ramWriter.io.word := in.bits.write_info.mem_word
  ramWriter.io.is_word := in.bits.controls.is_ram_word
  ramWriter.io.is_half := in.bits.controls.is_ram_half
  ramWriter.io.is_byte := in.bits.controls.is_ram_byte
  ramWriter.io.lower2bit := in.bits.write_info.alu_out(1, 0)

  fetch_port.reqValid := should_mem_access && state === sIDLE

  fetch_port.raddr := Cat(
    in.bits.write_info.alu_out(31, 2),
    "b00".U(2.W)
  )
  should_mem_access := in.bits.controls.is_ram_valid && in.valid

  fetch_port.wdata := in.bits.write_info.mem_word
  fetch_port.waddr := in.bits.write_info.alu_out
  fetch_port.wen := in.bits.controls.is_ram_wen && in.valid && state === sIDLE
  fetch_port.wmask := ramWriter.io.mask

  out.bits.pc := in.bits.pc
  out.bits.controls := in.bits.controls
  out.bits.itype := in.bits.itype

  out.bits.write_info := in.bits.write_info
  when(in.bits.controls.is_gpr_wdata_from_ram) {
    out.bits.write_info.gpr_wdata := ramLoader.io.out
  }

  out.valid := (in.valid && !in.bits.controls.is_ram_valid) || (fetch_port.respValid && state === sWAIT)
  in.ready := state === sWAIT && out.ready
}
