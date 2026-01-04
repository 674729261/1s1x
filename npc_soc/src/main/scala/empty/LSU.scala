package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageLSU2WBU extends Bundle {
  val pc = (UInt(32.W))
  val controls = (new ControlSignals)
  val itype = (new InstType)
  val write_info = (new WriteInfo)
}

class LSU() extends Module {

  val in = IO(Flipped(DecoupledIO(new MessageEXU2LSU)))

  val out = IO(DecoupledIO(new MessageLSU2WBU))

  val fetch_port = IO(new AXI)
  set_AXIfull_zero(fetch_port)

  val ramWriter = Module(new RamWriteData)
  val ramLoader = Module(new RamLoadData)

  val should_mem_access_r = Wire(Bool())
  val should_mem_access_w = Wire(Bool())

  val cpu_fire = out.valid && out.ready
  val aw_fire = fetch_port.aw.ready && fetch_port.aw.valid
  val w_fire = fetch_port.w.ready && fetch_port.w.valid
  val ar_fire = fetch_port.ar.ready && fetch_port.ar.valid
  val r_fire = fetch_port.r.ready && fetch_port.r.valid
  val b_fire = fetch_port.b.ready && fetch_port.b.valid

  val sIDLE_r :: sWAIT_RESP_r :: sWAIT_r :: Nil = Enum(3)
  val state_r = RegInit(sIDLE_r)
  state_r := MuxLookup(state_r, sIDLE_r)(
    Seq(
      sIDLE_r -> Mux(should_mem_access_r && ar_fire, sWAIT_RESP_r, sIDLE_r),
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

  fetch_port.ar.valid := should_mem_access_r && state_r === sIDLE_r
  fetch_port.ar.addr := in.bits.write_info.alu_out
  fetch_port.ar.size := Mux1H(
    Seq(
      in.bits.controls.is_ram_byte -> "b000".U(3.W),
      in.bits.controls.is_ram_half -> "b001".U(3.W),
      in.bits.controls.is_ram_word -> "b010".U(3.W)
    )
  )

  fetch_port.ar.id := "b1000".U(4.W)
  fetch_port.r.ready := state_r === sWAIT_RESP_r

  fetch_port.aw.addr := in.bits.write_info.alu_out
  fetch_port.aw.valid := should_mem_access_w && !out_aw
  fetch_port.aw.id := "b1000".U(4.W)

  fetch_port.w.data := ramWriter.io.out
  fetch_port.w.valid := should_mem_access_w && !out_w
  fetch_port.w.strb := ramWriter.io.mask
  fetch_port.aw.size := Mux1H(
    Seq(
      in.bits.controls.is_ram_byte -> "b000".U(3.W),
      in.bits.controls.is_ram_half -> "b001".U(3.W),
      in.bits.controls.is_ram_word -> "b010".U(3.W)
    )
  )
  fetch_port.w.last := true.B

  fetch_port.b.ready := out_aw && out_w && !has_b

  ramLoader.io.word := fetch_port.r.data
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

  block(AXIAssertLayer) {
    withDisable(Disable.Never) {
      when(fetch_port.ar.valid && !fetch_port.ar.ready) {
        assert(
          fetch_port.ar.valid === RegNext(fetch_port.ar.valid),
          "lsu.axi.arvalid dropped without handshake"
        )
      }
      when(fetch_port.w.valid && !fetch_port.w.ready) {
        assert(
          fetch_port.w.valid === RegNext(fetch_port.w.valid),
          "lsu.axi.wvalid dropped without handshake"
        )
      }
      when(fetch_port.aw.valid && !fetch_port.aw.ready) {
        assert(
          fetch_port.aw.valid === RegNext(fetch_port.aw.valid),
          "lsu.axi.awvalid dropped without handshake"
        )
      }
      when(r_fire) {
        assert(fetch_port.r.last, "lsu.axi.rlast is not set")
        assert(fetch_port.r.resp === "b00".U, "lsu.axi.rresp is not b00")
        assert(fetch_port.r.id === "b1000".U, "lsu.axi.rid is not b1000")
      }
      when(b_fire) {
        assert(fetch_port.b.resp === 0.U, "lsu.axi.bresp is not 0")
        assert(fetch_port.b.id === "b1000".U)
      }
    }
  }
}
