package empty.empty
import chisel3._
import chisel3.util._
import chisel3.util.random.LFSR
import empty.AXI_Lite

object set_flipped_AXI_zero {
  def apply(axi: AXI_Lite) = {
    axi.ar.arready := 0.U

    axi.r.rdata := 0.U
    axi.r.rresp := 0.U
    axi.r.rvalid := 0.U

    axi.aw.awready := 0.U

    axi.w.wready := 0.U

    axi.b.bresp := 0.U
    axi.b.bvalid := 0.U
  }
}

class __inst_fetch_bus() extends Module with RequireAsyncReset {
  val fetch_port = IO(Flipped(new AXI_Lite))
  set_flipped_AXI_zero(fetch_port)
  val io = IO(new Bundle {
    val addr = Output(UInt(32.W))
    val valid = Output(Bool())
    val instr = Input(UInt(32.W))
  })
  val sBUSY :: sWAIT_REQ :: sFETCH :: sWAIT_READY :: Nil = Enum(4)
  val state_next = Wire(UInt(sBUSY.getWidth.W))
  val state = RegEnable(state_next, sWAIT_REQ, true.B)

  val random_delay = Cat(0.U(29.W), LFSR(3))
  val counter_next = Wire(UInt(32.W))
  val counter_delay = RegEnable(counter_next, 0.U(32.W), true.B)

  state_next := MuxLookup(state, sWAIT_REQ)(
    Seq(
      sBUSY -> Mux(counter_delay === 0.U(32.W), sWAIT_REQ, sBUSY),
      sWAIT_REQ -> Mux(fetch_port.ar.arvalid, sFETCH, sWAIT_REQ),
      sFETCH -> Mux(counter_delay === 0.U(32.W), sWAIT_READY, sFETCH),
      sWAIT_READY -> Mux(fetch_port.r.rready, sBUSY, sWAIT_READY)
    )
  )

  counter_next := Mux(
    (state === sWAIT_READY && state_next === sBUSY) || (state === sWAIT_REQ && state_next === sFETCH),
    random_delay,
    Mux(
      counter_delay === 0.U(32.W),
      0.U(32.W),
      counter_delay - 1.U(32.W)
    )
  )

  fetch_port.ar.arready := state === sWAIT_REQ
  fetch_port.r.rvalid := state === sWAIT_READY

  io.addr := fetch_port.ar.araddr
  io.valid := state === sWAIT_REQ && state_next === sFETCH
  fetch_port.r.rdata := Mux(state === sWAIT_READY, io.instr, 0.U(32.W))

}
