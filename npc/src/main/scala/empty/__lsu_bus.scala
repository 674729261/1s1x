package empty.empty
import chisel3._
import chisel3.util._
import empty.InstBus
import _root_.empty.MemAccessBus
import chisel3.util.random.LFSR

class __lsu_fetch_bus() extends Module with RequireAsyncReset {
  val fetch_port = IO(Flipped(new MemAccessBus))
  val io = IO(new Bundle {
    val raddr = Output(UInt(32.W))
    val waddr = Output(UInt(32.W))
    val wdata = Output(UInt(32.W))
    val wmask = Output(UInt(4.W))
    val valid = Output(Bool())
    val wen = Output(Bool())
    val rdata = Input(UInt(32.W))
  })

  val sBUSY :: sWAIT_REQ :: sFETCH :: sWAIT_READY :: Nil = Enum(4)
  val state_next = Wire(UInt(sBUSY.getWidth.W))
  val state = RegEnable(state_next, sWAIT_REQ, true.B)

  val random_delay = Cat(0.U(28.W), LFSR(4))
  val counter_next = Wire(UInt(32.W))
  val counter_delay = RegEnable(counter_next, 0.U(32.W), true.B)

  state_next := MuxLookup(state, sWAIT_REQ)(
    Seq(
      sBUSY -> Mux(counter_delay === 0.U(32.W), sWAIT_REQ, sBUSY),
      sWAIT_REQ -> Mux(fetch_port.reqValid, sFETCH, sWAIT_REQ),
      sFETCH -> Mux(counter_delay === 0.U(32.W), sWAIT_READY, sFETCH),
      sWAIT_READY -> Mux(fetch_port.respReady, sBUSY, sWAIT_READY)
    )
  )

  counter_next := Mux(
    (state === sWAIT_READY && state_next === sBUSY) || (state === sWAIT_REQ && state_next === sFETCH),
    2.U,
    Mux(
      counter_delay === 0.U(32.W),
      0.U(32.W),
      counter_delay - 1.U(32.W)
    )
  )

  fetch_port.reqReady := state === sWAIT_REQ
  fetch_port.respValid := state === sWAIT_READY

  io.raddr := fetch_port.raddr
  io.waddr := fetch_port.waddr
  io.wen := fetch_port.wen
  io.wmask := fetch_port.wmask
  io.wdata := fetch_port.wdata
  io.valid := state === sWAIT_REQ && state_next === sFETCH

  fetch_port.rdata := Mux(state === sWAIT_READY, io.rdata, 0.U(32.W))
}
