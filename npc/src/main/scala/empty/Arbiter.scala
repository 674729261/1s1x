package empty
import chisel3._
import chisel3.util._

class FireSignal extends Bundle {
  val ar_fire = Bool()
  val r_fire = Bool()
  val r_burst_last = Bool()
  val aw_fire = Bool()
  val w_fire = Bool()
  val w_burst_last = Bool()
  val b_fire = Bool()
}
object GenerateFireSignal {
  def apply(axi: AXI): FireSignal = {
    val ret = Wire(new FireSignal)

    ret.ar_fire := axi.ar.valid && axi.ar.ready
    ret.r_fire := axi.r.ready && axi.r.valid
    ret.r_burst_last := axi.r.last && ret.r_fire
    ret.aw_fire := axi.aw.valid && axi.aw.ready
    ret.w_fire := axi.w.valid && axi.w.ready
    ret.w_burst_last := axi.w.last && ret.w_fire
    ret.b_fire := axi.b.ready && axi.b.valid

    return ret
  }
}

class Arbiter_2Master() extends Module {
  val IFU_AXI = IO(Flipped(new AXI))
  val LSU_AXI = IO(Flipped(new AXI))
  val OUT_AXI = IO(new AXI)

  set_flipped_AXIfull_zero(IFU_AXI)
  set_flipped_AXIfull_zero(LSU_AXI)
  set_AXIfull_zero(OUT_AXI)

  val IFU_fire = GenerateFireSignal(IFU_AXI)
  val LSU_fire = GenerateFireSignal(LSU_AXI)
  val out_fire = GenerateFireSignal(OUT_AXI)

  val sIDLE :: sIFU :: sLSU :: Nil = Enum(3)
  val ar_owner = RegInit(sIDLE)
  val r_owner = RegInit(sIDLE)
  val aw_w_owner = RegInit(sIDLE)
  val b_owner = RegInit(sIDLE)

  val bind_to_IFU_ar =
    (ar_owner === sIDLE && IFU_AXI.ar.valid) || ar_owner === sIFU
  val bind_to_LSU_ar =
    (ar_owner === sIDLE && LSU_AXI.ar.valid) || ar_owner === sLSU

  val bind_to_IFU_r =
    (r_owner === sIDLE && OUT_AXI.r.valid && OUT_AXI.r.id === "b0000".U(
      4.W
    )) || r_owner === sIFU
  val bind_to_LSU_r =
    (r_owner === sIDLE && OUT_AXI.r.valid && OUT_AXI.r.id === "b1000".U(
      4.W
    )) || r_owner === sLSU

  val bind_to_IFU_aw_w =
    (aw_w_owner === sIDLE && IFU_AXI.aw.valid) || aw_w_owner === sIFU
  val bind_to_LSU_aw_w =
    (aw_w_owner === sIDLE && LSU_AXI.aw.valid) || aw_w_owner === sLSU

  val bind_to_IFU_b =
    (b_owner === sIDLE && OUT_AXI.b.valid && OUT_AXI.b.id === "b0000".U(
      4.W
    )) || b_owner === sIFU
  val bind_to_LSU_b =
    (b_owner === sIDLE && OUT_AXI.b.valid && OUT_AXI.b.id === "b1000".U(
      4.W
    )) || b_owner === sLSU

  when(bind_to_IFU_ar) {
    IFU_AXI.ar <> OUT_AXI.ar
  }.elsewhen(bind_to_LSU_ar) {
    LSU_AXI.ar <> OUT_AXI.ar
  }

  when(bind_to_IFU_r) {
    IFU_AXI.r <> OUT_AXI.r
  }.elsewhen(bind_to_LSU_r) {
    LSU_AXI.r <> OUT_AXI.r
  }

  when(bind_to_IFU_aw_w) {
    IFU_AXI.w <> OUT_AXI.w
    IFU_AXI.aw <> OUT_AXI.aw
  }.elsewhen(bind_to_LSU_aw_w) {
    LSU_AXI.w <> OUT_AXI.w
    LSU_AXI.aw <> OUT_AXI.aw
  }

  when(bind_to_IFU_b) {
    IFU_AXI.b <> OUT_AXI.b
  }.elsewhen(bind_to_LSU_b) {
    LSU_AXI.b <> OUT_AXI.b
  }

  ar_owner := MuxLookup(ar_owner, sIDLE)(
    Seq(
      sIDLE -> Mux(
        IFU_AXI.ar.valid,
        sIFU,
        Mux(LSU_AXI.ar.valid, sLSU, sIDLE)
      ),
      sIFU -> Mux(IFU_fire.ar_fire, sIDLE, sIFU),
      sLSU -> Mux(LSU_fire.ar_fire, sIDLE, sLSU)
    )
  )

  r_owner := MuxLookup(r_owner, sIDLE)(
    Seq(
      sIDLE -> Mux(
        bind_to_IFU_r,
        sIFU,
        Mux(bind_to_LSU_r, sLSU, sIDLE)
      ),
      sIFU -> Mux(IFU_fire.r_burst_last, sIDLE, sIFU),
      sLSU -> Mux(LSU_fire.r_burst_last, sIDLE, sLSU)
    )
  )

  aw_w_owner := MuxLookup(aw_w_owner, sIDLE)(
    Seq(
      sIDLE -> Mux(
        bind_to_IFU_aw_w,
        sIFU,
        Mux(bind_to_LSU_aw_w, sLSU, sIDLE)
      ),
      sIFU -> Mux(IFU_fire.w_burst_last, sIDLE, sIFU),
      sLSU -> Mux(LSU_fire.w_burst_last, sIDLE, sLSU)
    )
  )

  b_owner := MuxLookup(b_owner, sIDLE)(
    Seq(
      sIDLE -> Mux(
        bind_to_IFU_b,
        sIFU,
        Mux(bind_to_LSU_b, sLSU, sIDLE)
      ),
      sIFU -> Mux(IFU_fire.b_fire, sIDLE, sIFU),
      sLSU -> Mux(LSU_fire.b_fire, sIDLE, sLSU)
    )
  )
}
