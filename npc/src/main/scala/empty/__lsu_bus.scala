package empty
import chisel3._
import chisel3.util._
import chisel3.util.random.LFSR

class __lsu_fetch_bus() extends Module with RequireAsyncReset {
  val fetch_port = IO(Flipped(new AXI_Lite))
  val io = IO(new Bundle {
    val raddr = Output(UInt(32.W))
    val waddr = Output(UInt(32.W))
    val wdata = Output(UInt(32.W))
    val wmask = Output(UInt(4.W))
    val valid = Output(Bool())
    val wen = Output(Bool())
    val rdata = Input(UInt(32.W))
  })

  val random_delay = LFSR(4)

  val ar_timeup = random_delay === 2.U(4.W)
  val r_timeup = random_delay === 3.U(4.W)
  val aw_timeup = random_delay === 4.U(4.W)
  val w_timeup = random_delay === 5.U(4.W)
  val b_timeup = random_delay === 6.U(4.W)

  val ar_fire = fetch_port.ar.arvalid && fetch_port.ar.arready
  val r_fire = fetch_port.r.rready && fetch_port.r.rvalid

  val aw_fire = fetch_port.aw.awvalid && fetch_port.aw.awready
  val w_fire = fetch_port.w.wvalid && fetch_port.w.wready
  val b_fire = fetch_port.b.bready && fetch_port.b.bvalid

  val sIDLE_r :: sDELAY_r :: sWAIT_r :: sDELAY2_r :: Nil = Enum(4)
  val state_r = RegInit(sIDLE_r)
  state_r := MuxLookup(state_r, sIDLE_r)(
    Seq(
      sIDLE_r -> Mux(ar_fire, sDELAY_r, sIDLE_r),
      sDELAY_r -> Mux(ar_timeup, sWAIT_r, sDELAY_r),
      sWAIT_r -> Mux(r_fire, sDELAY2_r, sWAIT_r),
      sDELAY2_r -> Mux(r_timeup, sIDLE_r, sDELAY2_r)
    )
  )

  val has_aw = RegInit(false.B)
  val has_w = RegInit(false.B)
  val do_response = RegInit(false.B)

  val wdata_reg = RegEnable(fetch_port.w.wdata, w_fire)
  val waddr_reg = RegEnable(fetch_port.aw.awaddr, aw_fire)
  val wstrb_reg = RegEnable(fetch_port.w.wstrb, w_fire)

  val response = has_aw && has_w && !fetch_port.b.bvalid && b_timeup

  has_aw := MuxCase(
    has_aw,
    Seq(
      aw_fire -> true.B,
      response -> false.B
    )
  )
  has_w := MuxCase(
    has_w,
    Seq(
      w_fire -> true.B,
      response -> false.B
    )
  )
  do_response := MuxCase(
    do_response,
    Seq(
      response -> true.B,
      b_fire -> false.B
    )
  )

  fetch_port.ar.arready := state_r === sIDLE_r
  fetch_port.r.rvalid := state_r === sWAIT_r
  fetch_port.r.rdata := io.rdata
  fetch_port.r.rresp := "b00".U(2.W)
  fetch_port.aw.awready := !has_aw
  fetch_port.w.wready := !has_w
  fetch_port.b.bvalid := do_response
  fetch_port.b.bresp := "b00".U(2.W)

  io.raddr := RegEnable(fetch_port.ar.araddr, ar_fire)
  io.waddr := RegEnable(fetch_port.aw.awaddr, aw_fire)
  io.wdata := RegEnable(fetch_port.w.wdata, w_fire)
  io.wmask := RegEnable(fetch_port.w.wstrb, w_fire)
  io.valid := (state_r === sDELAY_r && ar_timeup) || (response)
  io.wen := response
}
