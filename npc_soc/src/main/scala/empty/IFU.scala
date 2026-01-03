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

  val sIDLE :: sWAIT_RESP :: sWAIT :: Nil = Enum(3)

  val state = RegInit(sIDLE)

  val ar_fire = fetch_port.ar.valid && fetch_port.ar.ready
  val r_fire = fetch_port.r.valid && fetch_port.r.ready

  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(ar_fire, sWAIT_RESP, sIDLE),
      sWAIT_RESP -> Mux(
        r_fire,
        Mux(out.ready, sIDLE, sWAIT),
        sWAIT_RESP
      ),
      sWAIT -> Mux(out.ready, sIDLE, sWAIT)
    )
  )

  val inst_reg =
    RegEnable(fetch_port.r.data, r_fire)

  fetch_port.ar.addr := Mux(state === sIDLE, in.pc, 12345.U(32.W))
  fetch_port.ar.valid := state === sIDLE
  fetch_port.r.ready := state === sWAIT_RESP
  fetch_port.aw.id := "b0000".U(4.W)
  fetch_port.ar.id := "b0000".U(4.W)
  fetch_port.w.last := true.B
  out.valid := state === sWAIT || r_fire

  out.bits.inst := Mux(state === sWAIT_RESP, fetch_port.r.data, inst_reg)
  out.bits.pc := in.pc

  block(AXIAssertLayer) {
    withDisable(Disable.Never) {
      when(fetch_port.ar.valid && !fetch_port.ar.ready) {
        assert(
          fetch_port.ar.valid === RegNext(fetch_port.ar.valid),
          "ifu.axi.arvalid dropped without handshake"
        )
      }
      when(r_fire) {
        assert(fetch_port.r.last, "ifu.axi.rlast is not set")
        assert(fetch_port.r.resp === "b00".U, "ifu.axi.rresp is not b00")
        assert(fetch_port.r.id === "b0000".U, "ifu.axi.rid is not b0000")
      }
    }
  }
}
