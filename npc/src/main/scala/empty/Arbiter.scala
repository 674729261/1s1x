package empty
import chisel3._
import chisel3.util._

class FireSignal extends Bundle {
  val ar_fire = Bool()
  val r_fire = Bool()
  val aw_fire = Bool()
  val w_fire = Bool()
  val b_fire = Bool()
}
object GenrageFireSignal {
  def apply(axi: AXI_Lite): FireSignal = {
    val ret = Wire(new FireSignal)

    ret.ar_fire := axi.ar.arvalid && axi.ar.arready
    ret.r_fire := axi.r.rready && axi.r.rvalid
    ret.aw_fire := axi.aw.awvalid && axi.aw.awready
    ret.w_fire := axi.w.wvalid && axi.w.wready
    ret.b_fire := axi.b.bready && axi.b.bvalid

    return ret
  }
}

class Arbiter_2Master() extends Module with RequireAsyncReset {
  val IFU_AXI = IO(Flipped(new AXI_Lite))
  val LSU_AXI = IO(Flipped(new AXI_Lite))
  val OUT_AXI = IO(new AXI_Lite)

  set_flipped_AXI_zero(IFU_AXI)
  set_flipped_AXI_zero(LSU_AXI)
  set_AXI_zero(OUT_AXI)

  val IFU_fire = GenrageFireSignal(IFU_AXI)
  val LSU_fire = GenrageFireSignal(LSU_AXI)
  val out_fire = GenrageFireSignal(OUT_AXI)

  val any_ar_fire = IFU_fire.ar_fire || LSU_fire.ar_fire
  val any_r_fire = IFU_fire.r_fire || LSU_fire.r_fire
  val any_aw_fire = IFU_fire.aw_fire || LSU_fire.aw_fire
  val any_w_fire = IFU_fire.w_fire || LSU_fire.w_fire
  val any_b_fire = IFU_fire.b_fire || LSU_fire.b_fire

  val sIDLE :: sIFU :: sLSU :: Nil = Enum(3)
  val r_owner = RegInit(sIDLE)
  val w_owner = RegInit(sIDLE)

  when((r_owner === sIDLE && IFU_AXI.ar.arvalid) || r_owner === sIFU) {
    IFU_AXI.ar <> OUT_AXI.ar
    IFU_AXI.r <> OUT_AXI.r
  }
  when((r_owner === sIDLE && LSU_AXI.ar.arvalid) || r_owner === sLSU) {
    LSU_AXI.ar <> OUT_AXI.ar
    LSU_AXI.r <> OUT_AXI.r
  }

  r_owner := MuxLookup(r_owner, sIDLE)(
    Seq(
      sIDLE -> Mux(
        IFU_AXI.ar.arvalid,
        sIFU,
        Mux(LSU_AXI.ar.arvalid, sLSU, sIDLE)
      ),
      sIFU -> Mux(IFU_fire.r_fire, sIDLE, sIFU),
      sLSU -> Mux(LSU_fire.r_fire, sIDLE, sLSU)
    )
  )

  when(
    (w_owner === sIDLE && (IFU_AXI.aw.awvalid || IFU_AXI.w.wvalid)) || w_owner === sIFU
  ) {
    IFU_AXI.aw <> OUT_AXI.aw
    IFU_AXI.w <> OUT_AXI.w
    IFU_AXI.b <> OUT_AXI.b
  }
  when(
    (w_owner === sIDLE && (LSU_AXI.aw.awvalid || LSU_AXI.w.wvalid)) || w_owner === sLSU
  ) {
    LSU_AXI.aw <> OUT_AXI.aw
    LSU_AXI.w <> OUT_AXI.w
    LSU_AXI.b <> OUT_AXI.b
  }

  w_owner := MuxLookup(w_owner, sIDLE)(
    Seq(
      sIDLE -> Mux(
        IFU_AXI.aw.awvalid || IFU_AXI.w.wvalid,
        sIFU,
        Mux(LSU_AXI.aw.awvalid || LSU_AXI.w.wvalid, sLSU, sIDLE)
      ),
      sIFU -> Mux(IFU_fire.b_fire, sIDLE, sIFU),
      sLSU -> Mux(LSU_fire.b_fire, sIDLE, sLSU)
    )
  )
}
