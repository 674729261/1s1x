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

  val cpu_out_fire = out.valid && out.ready
  val cpu_in_fire = in.valid && in.ready

  val aw_fire = fetch_port.aw.ready && fetch_port.aw.valid
  val w_fire = fetch_port.w.ready && fetch_port.w.valid
  val ar_fire = fetch_port.ar.ready && fetch_port.ar.valid
  val r_fire = fetch_port.r.ready && fetch_port.r.valid
  val b_fire = fetch_port.b.ready && fetch_port.b.valid

  val should_signal_in_latch = in.valid

  fetch_port.aw.id := "b1000".U(4.W)
  fetch_port.ar.id := "b1000".U(4.W)
  fetch_port.w.last := true.B

  val signal_in_r = RegEnable(in.bits, should_signal_in_latch)
  val has_signal = RegInit(false.B)
  has_signal := MuxCase(
    has_signal,
    Seq(
      should_signal_in_latch -> true.B,
      cpu_out_fire -> false.B
    )
  )

  val out_ar = RegInit(false.B)
  out_ar := MuxCase(
    out_ar,
    Seq(
      ar_fire -> true.B,
      r_fire -> false.B
    )
  )
  val has_r = RegInit(false.B)
  has_r := MuxCase(
    has_r,
    Seq(
      r_fire -> true.B,
      cpu_out_fire -> false.B
    )
  )
  val out_aw = RegInit(false.B)
  val out_w = RegInit(false.B)
  out_aw := MuxCase(
    out_aw,
    Seq(
      aw_fire -> true.B,
      b_fire -> false.B
    )
  )
  out_w := MuxCase(
    out_w,
    Seq(
      w_fire -> true.B,
      b_fire -> false.B
    )
  )
  val has_b = RegInit(false.B)
  has_b := MuxCase(
    has_b,
    Seq(
      b_fire -> true.B,
      cpu_out_fire -> false.B
    )
  )
  should_mem_access_r := has_signal && signal_in_r.controls.is_ram_valid && !signal_in_r.controls.is_ram_wen
  should_mem_access_w := has_signal && signal_in_r.controls.is_ram_valid && signal_in_r.controls.is_ram_wen

  fetch_port.ar.valid := has_signal && should_mem_access_r && !out_ar
  fetch_port.r.ready := out_ar && !has_r
  fetch_port.aw.valid := has_signal && should_mem_access_w && !out_aw
  fetch_port.w.valid := has_signal && should_mem_access_w && !out_w
  fetch_port.b.ready := out_aw && out_w && !has_b

  ramLoader.io.word := fetch_port.r.data
  ramLoader.io.is_byte := signal_in_r.controls.is_ram_byte
  ramLoader.io.is_half := signal_in_r.controls.is_ram_half
  ramLoader.io.is_word := signal_in_r.controls.is_ram_word
  ramLoader.io.is_unsigned := signal_in_r.controls.is_load_unsigned
  ramLoader.io.lower2bit := signal_in_r.write_info.alu_out(1, 0)

  ramWriter.io.word := signal_in_r.write_info.mem_word
  ramWriter.io.is_word := signal_in_r.controls.is_ram_word
  ramWriter.io.is_half := signal_in_r.controls.is_ram_half
  ramWriter.io.is_byte := signal_in_r.controls.is_ram_byte
  ramWriter.io.lower2bit := signal_in_r.write_info.alu_out(1, 0)

  val rdata_latched = RegEnable(ramLoader.io.out, r_fire)

  out.bits.pc := signal_in_r.pc
  out.bits.controls := signal_in_r.controls
  out.bits.itype := signal_in_r.itype
  out.bits.write_info := signal_in_r.write_info
  when(signal_in_r.controls.is_gpr_wdata_from_ram) {
    out.bits.write_info.gpr_wdata := rdata_latched
  }

  out.valid := has_signal && ((should_mem_access_r && has_r) || (should_mem_access_w && has_b) || (!should_mem_access_r && !should_mem_access_w))
  in.ready := out.ready
  block(AXIAssertLayer) {
    check_signal_stable(
      fetch_port.ar.ready,
      fetch_port.ar.valid,
      Cat(
        fetch_port.ar.addr,
        fetch_port.ar.burst,
        fetch_port.ar.id,
        fetch_port.ar.len,
        fetch_port.ar.size
      ),
      "IFU.ar"
    )
    check_signal_stable(
      fetch_port.w.ready,
      fetch_port.w.valid,
      Cat(
        fetch_port.w.data,
        fetch_port.w.last,
        fetch_port.w.strb
      ),
      "IFU.w"
    )
    check_signal_stable(
      fetch_port.aw.ready,
      fetch_port.aw.valid,
      Cat(
        fetch_port.aw.addr,
        fetch_port.aw.burst,
        fetch_port.aw.id,
        fetch_port.aw.len,
        fetch_port.aw.size
      ),
      "IFU.aw"
    )
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
