package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
}

class IFU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
  })

  val fetch_port = IO(new AXI_Lite)
  set_AXI_zero(fetch_port)
  val out = IO(DecoupledIO(new MessageIFU2IDU))

  val sIDLE :: sWAIT_RESP :: sWAIT :: Nil = Enum(3)

  val state = RegInit(sIDLE)
  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(fetch_port.ar.arready, sWAIT_RESP, sIDLE),
      sWAIT_RESP -> Mux(
        fetch_port.r.rvalid,
        Mux(out.ready, sIDLE, sWAIT),
        sWAIT_RESP
      ),
      sWAIT -> Mux(out.ready, sIDLE, sWAIT)
    )
  )

  val inst_reg =
    RegEnable(fetch_port.r.rdata, state === sWAIT_RESP && fetch_port.r.rvalid)

  fetch_port.ar.araddr := Mux(state === sIDLE, in.pc, 12345.U(32.W))
  fetch_port.ar.arvalid := state === sIDLE
  fetch_port.r.rready := state === sWAIT_RESP
  out.valid := state === sWAIT || (state === sWAIT_RESP && fetch_port.r.rvalid)

  out.bits.inst := Mux(state === sWAIT_RESP, fetch_port.r.rdata, inst_reg)
  out.bits.pc := in.pc

  block(AXIAssertLayer) {
    withDisable(Disable.Never) {
      when(fetch_port.ar.arvalid && !fetch_port.ar.arready) {
        assert(
          fetch_port.ar.arvalid === RegNext(fetch_port.ar.arvalid),
          "ifu.axi.ar.arvalid dropped without handshake"
        )
      }
    }
  }
}
