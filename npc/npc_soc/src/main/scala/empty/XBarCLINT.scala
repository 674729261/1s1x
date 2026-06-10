package empty

import chisel3._
import chisel3.util._
import chisel3.layer._

class XBar_CLINT() extends PrefixedModule {
  val IN_AXI = IO(Flipped(new AXI))
  val OUT_AXI = IO(new AXI)
  val CLINT_AXI = IO(new AXI)

  set_flipped_AXIfull_zero(IN_AXI)
  set_AXIfull_zero(OUT_AXI)
  set_AXIfull_zero(CLINT_AXI)

  val IN_fire = GenerateFireSignal(IN_AXI)
  val OUT_fire = GenerateFireSignal(OUT_AXI)
  val CLINT_fire = GenerateFireSignal(CLINT_AXI)

  val sel_clint_ar = IN_AXI.ar.addr(31, 24) === 0x02.U(8.W)
  when(sel_clint_ar) {
    CLINT_AXI.ar <> IN_AXI.ar
  }.otherwise {
    OUT_AXI.ar <> IN_AXI.ar
  }

  IN_AXI.aw <> OUT_AXI.aw
  IN_AXI.w <> OUT_AXI.w
  IN_AXI.b <> OUT_AXI.b

  val sOUT :: sCLINT :: Nil = Enum(2)
  val r_state = RegInit(sOUT)
  r_state := MuxLookup(r_state, sOUT)(
    Seq(
      sOUT -> Mux(CLINT_AXI.r.valid && !OUT_AXI.r.valid, sCLINT, sOUT),
      sCLINT -> Mux(CLINT_fire.r_burst_last, sOUT, sCLINT),
    )
  )

  val should_bind_to_OUT_r = r_state === sOUT
  when(should_bind_to_OUT_r) {
    OUT_AXI.r <> IN_AXI.r
  }.otherwise {
    CLINT_AXI.r <> IN_AXI.r
  }
  val ext_ar_addr_hold = RegInit(0.U(32.W))
  val ext_ar_pending = RegInit(false.B)
  when(OUT_fire.ar_fire) {
    ext_ar_addr_hold := OUT_AXI.ar.addr
    ext_ar_pending := true.B
  }.elsewhen(OUT_fire.r_burst_last) {
    ext_ar_pending := false.B
  }
  when(ext_ar_pending && !OUT_AXI.ar.valid) {
    OUT_AXI.ar.addr := ext_ar_addr_hold
  }

  val ext_aw_addr_hold = RegInit(0.U(32.W))
  val ext_aw_pending = RegInit(false.B)
  when(OUT_fire.aw_fire) {
    ext_aw_addr_hold := OUT_AXI.aw.addr
    ext_aw_pending := true.B
  }.elsewhen(OUT_fire.b_fire) {
    ext_aw_pending := false.B
  }
  when(ext_aw_pending && !OUT_AXI.aw.valid) {
    OUT_AXI.aw.addr := ext_aw_addr_hold
  }
}

class Clint extends PrefixedModule {
  val in = IO(Flipped(new AXI))
  val mtime = IO(Input(UInt(40.W)))

  set_flipped_AXIfull_zero(in)

  val fire = GenerateFireSignal(in)
  val has_ar = RegInit(false.B)
  val low_or_high = RegInit(false.B)

  when(fire.ar_fire) {
    low_or_high := in.ar.addr(2)
  }

  has_ar := MuxCase(
    has_ar,
    Seq(
      fire.ar_fire -> true.B,
      fire.r_burst_last -> false.B,
    )
  )

  in.ar.ready := !has_ar
  in.r.valid := has_ar
  in.r.data := Mux(low_or_high, Cat(0.U(24.W), mtime(39, 32)), mtime(31, 0))
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
