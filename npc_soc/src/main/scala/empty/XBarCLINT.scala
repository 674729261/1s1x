package empty
import chisel3._
import chisel3.util._
import chisel3.layer._

class XBar_CLINT() extends Module {
  val IN_AXI = IO(Flipped(new AXI))
  val OUT_AXI = IO(new AXI)
  val CLINT_AXI = IO(new AXI)

  set_flipped_AXIfull_zero(IN_AXI)
  set_AXIfull_zero(OUT_AXI)
  set_AXIfull_zero(CLINT_AXI)

  val IN_fire = GenerateFireSignal(IN_AXI)
  val OUT_fire = GenerateFireSignal(OUT_AXI)
  val CLINT_fire = GenerateFireSignal(CLINT_AXI)

  val sel_clint_ar = (IN_AXI.ar.addr(31, 24) === 0x02.U(8.W))
  when(sel_clint_ar) {
    CLINT_AXI.ar <> IN_AXI.ar
  }.otherwise { OUT_AXI.ar <> IN_AXI.ar }

  IN_AXI.aw <> OUT_AXI.aw
  IN_AXI.w <> OUT_AXI.w
  IN_AXI.b <> OUT_AXI.b

  val sIDLE :: sOUT :: sCLINT :: Nil = Enum(3)
  val r_state = RegInit(sIDLE)
  r_state := MuxLookup(r_state, sIDLE)(
    Seq(
      sIDLE -> MuxCase(
        sIDLE,
        Seq(OUT_AXI.r.valid -> sOUT, CLINT_AXI.r.valid -> sCLINT)
      ),
      sOUT -> Mux(OUT_fire.r_burst_last, sIDLE, sOUT),
      sCLINT -> Mux(CLINT_fire.r_burst_last, sIDLE, sCLINT)
    )
  )
  when(r_state === sIDLE || r_state === sOUT) {
    OUT_AXI.r <> IN_AXI.r
  }.otherwise {
    CLINT_AXI.r <> IN_AXI.r
  }

}

class Clint extends Module {
  val in = IO(Flipped(new AXI))
  set_flipped_AXIfull_zero(in)
  val fire = GenerateFireSignal(in)

  val mtime = RegInit(Vec(2, UInt(32.W)), VecInit(Seq(0.U(32.W), 0.U(32.W))))
  mtime := (mtime.asUInt + 1.U(64.W)).asTypeOf(Vec(2, UInt(32.W)))

  val has_ar = RegInit(Bool(), false.B)
  val out_r = RegInit(Bool(), false.B)
  val low_or_high = RegEnable(in.ar.addr(2), fire.ar_fire)
  has_ar := MuxCase(
    has_ar,
    Seq(
      fire.ar_fire -> true.B,
      fire.r_burst_last -> false.B
    )
  )
  in.ar.ready := !has_ar
  in.r.valid := has_ar
  in.r.data := Mux(low_or_high, mtime(1), mtime(0))
  in.r.last := true.B
  in.r.id := "b1000".U(4.W)

  block(Verifying) {
    when(fire.ar_fire) {
      assert(in.ar.id === "b1000".U(4.W), "CLINT can only be accessed by LSU")
      assert(in.ar.len === 0.U, "Burst length can only be 1")
    }
    assert(!in.aw.valid, "CLINT can not be written")
    assert(!in.w.valid, "CLINT can not be written")

  }

}
