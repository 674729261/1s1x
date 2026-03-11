package ysyx

import chisel3._
import chisel3.util._

import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.amba._
import freechips.rocketchip.amba.axi4._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._

class AXI4DelayerIO extends Bundle {
  val clock = Input(Clock())
  val reset = Input(Reset())
  val in = Flipped(
    new AXI4Bundle(
      AXI4BundleParameters(addrBits = 32, dataBits = 32, idBits = 4)
    )
  )
  val out = new AXI4Bundle(
    AXI4BundleParameters(addrBits = 32, dataBits = 32, idBits = 4)
  )
}

class axi4_delayer extends BlackBox {
  val io = IO(new AXI4DelayerIO)
}

class AXI4DelayerChisel(ratio: Double = 19.89032, scale_2pow: Long = 6)
    extends Module {
  val io = IO(new AXI4DelayerIO)
  val countup_amount = math.round((ratio - 1.0) * math.pow(2.0, scale_2pow))
  val countdown_amount = (1 << scale_2pow)
  io.out <> io.in
  val sIDLE :: sDELAY :: sWAIT :: Nil = Enum(3)
  val state_ar = RegInit(sIDLE)
  val trigger_ar = io.in.ar.valid
  val fire_ar = io.in.ar.valid && io.in.ar.ready
  val counter_ar = RegInit(UInt(32.W), 0.U)

  io.out.ar.valid := state_ar === sWAIT

  state_ar := MuxLookup(state_ar, sDELAY)(
    Seq(
      sIDLE -> Mux(trigger_ar, sDELAY, sIDLE),
      sDELAY -> Mux(counter_ar < countdown_amount.U, sWAIT, sDELAY),
      sWAIT -> Mux(fire_ar, sIDLE, sWAIT)
    )
  )

  counter_ar := MuxCase(
    counter_ar,
    Seq(
      (state_ar === sWAIT && !fire_ar) -> (counter_ar + countup_amount.U),
      (state_ar === sDELAY && counter_ar >= countdown_amount.U) -> (counter_ar - countdown_amount.U)
    )
  )

  val state_aw = RegInit(sIDLE)
  val trigger_aw = io.in.aw.valid
  val fire_aw = io.in.aw.valid && io.in.aw.ready
  val counter_aw = RegInit(UInt(32.W), 0.U)

  io.out.aw.valid := state_aw === sWAIT

  state_aw := MuxLookup(state_aw, sDELAY)(
    Seq(
      sIDLE -> Mux(trigger_aw, sDELAY, sIDLE),
      sDELAY -> Mux(counter_aw < countdown_amount.U, sWAIT, sDELAY),
      sWAIT -> Mux(fire_aw, sIDLE, sWAIT)
    )
  )

  counter_aw := MuxCase(
    counter_aw,
    Seq(
      (state_aw === sWAIT && !fire_aw) -> (counter_aw + countup_amount.U),
      (state_aw === sDELAY && counter_aw >= countdown_amount.U) -> (counter_aw - countdown_amount.U)
    )
  )

}

class AXI4DelayerWrapper(implicit p: Parameters) extends LazyModule {
  val node = AXI4IdentityNode()

  lazy val module = new Impl
  class Impl extends LazyModuleImp(this) {
    (node.in zip node.out) foreach { case ((in, edgeIn), (out, edgeOut)) =>
      val delayer = Module(new AXI4DelayerChisel)
      delayer.io.clock := clock
      delayer.io.reset := reset
      delayer.io.in <> in
      out <> delayer.io.out
    }
  }
}

object AXI4Delayer {
  def apply()(implicit p: Parameters): AXI4Node = {
    val axi4delay = LazyModule(new AXI4DelayerWrapper)
    axi4delay.node
  }
}
