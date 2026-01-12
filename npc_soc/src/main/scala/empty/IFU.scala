package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
}

class IFU() extends Module {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
  })

  val fetch_port = IO(new AXI)
  set_AXIfull_zero(fetch_port)
  val out = IO(DecoupledIO(new MessageIFU2IDU))

  val ar_fire = fetch_port.ar.valid && fetch_port.ar.ready
  val r_fire = fetch_port.r.valid && fetch_port.r.ready

  val out_ar = RegInit(Bool(), false.B)
  val has_r = RegInit(Bool(), false.B)
  val cpu_out_fire = out.valid && out.ready

  out_ar := MuxCase(
    out_ar,
    Seq(
      ar_fire -> true.B,
      cpu_out_fire -> false.B
    )
  )

  has_r := MuxCase(
    has_r,
    Seq(
      r_fire -> true.B,
      cpu_out_fire -> false.B
    )
  )

  fetch_port.ar.valid := !out_ar
  fetch_port.r.ready := !has_r
  val inst_reg =
    RegEnable(fetch_port.r.data, r_fire)

  fetch_port.ar.addr := in.pc

  fetch_port.aw.id := "b0000".U(4.W)
  fetch_port.ar.id := "b0000".U(4.W)
  fetch_port.w.last := true.B
  out.valid := has_r

  out.bits.inst := inst_reg
  out.bits.pc := in.pc

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
    assert(!fetch_port.aw.valid && !fetch_port.w.valid, "ifu should not write")
    when(r_fire) {
      assert(fetch_port.r.last, "ifu.axi.rlast is not set")
      assert(fetch_port.r.resp === "b00".U, "ifu.axi.rresp is not b00")
      assert(fetch_port.r.id === "b0000".U, "ifu.axi.rid is not b0000")

    }
  }
}
