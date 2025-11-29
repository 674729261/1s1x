package empty.empty
import chisel3._
import chisel3.util._
import empty.RamWriteData
import empty.RamLoadData
import empty.MemAccessBus
import svsim.CommonCompilationSettings.Timescale.Unit.s
import empty.AXI_Lite

class MessageLSU2WBU extends Bundle {
  val pc = (UInt(32.W))
  val controls = (new ControlSignals)
  val itype = (new InstType)
  val write_info = (new WriteInfo)
}

class LSU() extends Module with RequireAsyncReset {

  val in = IO(Flipped(DecoupledIO(new MessageEXU2LSU)))

  val out = IO(DecoupledIO(new MessageLSU2WBU))

  val fetch_port = IO(new AXI_Lite)

  val ramWriter = Module(new RamWriteData)
  val ramLoader = Module(new RamLoadData)

  val should_mem_access_r = Wire(Bool())
  val should_mem_access_w = Wire(Bool())

  val cpu_fire = out.valid && out.ready
  val aw_fire = fetch_port.aw.awready && fetch_port.aw.awvalid
  val w_fire = fetch_port.w.wready && fetch_port.w.wvalid
  val ar_fire = fetch_port.ar.arready && fetch_port.ar.arvalid
  val r_fire = fetch_port.r.rready && fetch_port.r.rvalid
  val b_fire = fetch_port.b.bready && fetch_port.b.bvalid

  val sIDLE_r :: sWAIT_RESP_r :: sWAIT_r :: Nil = Enum(3)
  val state_r = RegInit(sIDLE_r)
  state_r := MuxLookup(state_r, sIDLE_r)(
    Seq(
      sIDLE_r -> Mux(should_mem_access_r && ar_fire, sWAIT_r, sIDLE_r),
      sWAIT_RESP_r -> Mux(r_fire, sWAIT_r, sWAIT_RESP_r),
      sWAIT_r -> Mux(cpu_fire, sIDLE_r, sWAIT_r)
    )
  )

  val out_aw = RegInit(false.B)
  val out_w = RegInit(false.B)
  val has_b = RegInit(false.B)
  out_aw := MuxCase(
    out_aw,
    Seq(
      aw_fire -> true.B,
      cpu_fire -> false.B
    )
  )
  out_w := MuxCase(
    out_w,
    Seq(
      w_fire -> true.B,
      cpu_fire -> false.B
    )
  )
  has_b := MuxCase(
    has_b,
    Seq(
      b_fire -> true.B,
      cpu_fire -> false.B
    )
  )

  val rdata_reg = RegEnable(ramLoader.io.out, r_fire)

  fetch_port.ar.arvalid := should_mem_access_r && state_r === sIDLE_r
  fetch_port.ar.araddr := Cat(
    in.bits.write_info.alu_out(31, 2),
    "b00".U(2.W)
  )

  fetch_port.r.rready := state_r === sWAIT_RESP_r

  fetch_port.aw.awaddr := in.bits.write_info.alu_out
  fetch_port.aw.awvalid := should_mem_access_w && !out_aw

  fetch_port.w.wdata := ramWriter.io.out
  fetch_port.w.wvalid := should_mem_access_w && !out_w
  fetch_port.w.wstrb := ramWriter.io.mask

  fetch_port.b.bready := out_aw && out_w && !has_b

  ramLoader.io.word := fetch_port.r.rdata
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

  should_mem_access_r := in.bits.controls.is_ram_valid && !in.bits.controls.is_ram_wen && in.valid
  should_mem_access_w := in.bits.controls.is_ram_valid && in.bits.controls.is_ram_wen && in.valid

  out.bits.pc := in.bits.pc
  out.bits.controls := in.bits.controls
  out.bits.itype := in.bits.itype

  out.bits.write_info := in.bits.write_info
  when(in.bits.controls.is_gpr_wdata_from_ram) {
    out.bits.write_info.gpr_wdata := rdata_reg
  }

  val no_mem_access = in.valid && !in.bits.controls.is_ram_valid
  val load_finished = state_r === sWAIT_r
  val save_finished = out_aw && out_w && has_b

  out.valid := no_mem_access || (should_mem_access_r && load_finished) || (should_mem_access_w && save_finished)
  in.ready := cpu_fire
}
