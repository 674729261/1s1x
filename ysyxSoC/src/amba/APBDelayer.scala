package ysyx

import chisel3._
import chisel3.util._

import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.amba._
import freechips.rocketchip.amba.apb._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._

class APBDelayerIO extends Bundle {
  val clock = Input(Clock())
  val reset = Input(Reset())
  val in = Flipped(
    new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32))
  )
  val out = new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32))
}

class apb_delayer extends BlackBox {
  val io = IO(new APBDelayerIO)
}

class APBDelayerChisel(ratio: Double = 6, scale_2pow: Long = 6) extends Module {
  val io = IO(new APBDelayerIO)
  val countup_amount = math.round((ratio - 1.0) * math.pow(2.0, scale_2pow))
  val countdown_amount = (1 << scale_2pow)
  val counter = RegInit(UInt(32.W), 0.U)
  val sIDLE :: sWAIT :: sDELAY :: sRESP :: Nil = Enum(4)
  val state = RegInit(sIDLE)

  val trigger = io.in.penable && io.in.psel
  val out_fire = io.out.psel && io.out.penable && io.out.pready
  state := MuxLookup(state, sDELAY)(
    Seq(
      sIDLE -> Mux(trigger, Mux(out_fire, sDELAY, sWAIT), sIDLE),
      sWAIT -> Mux(out_fire, sDELAY, sWAIT),
      sDELAY -> Mux(counter < countdown_amount.U, sRESP, sDELAY),
      sRESP -> sIDLE
    )
  )
  counter := MuxCase(
    counter,
    Seq(
      (state === sWAIT && !out_fire) -> (counter + countup_amount.U),
      (state === sDELAY && counter >= countdown_amount.U) -> (counter - countdown_amount.U)
    )
  )

  val prdata_r = RegEnable(io.out.prdata, out_fire)
  val pslverr_r = RegEnable(io.out.pslverr, out_fire)

  io.out <> io.in
  io.in.prdata := prdata_r
  io.in.pslverr := pslverr_r
  io.out.penable := Mux(
    state === sIDLE || state === sWAIT,
    io.in.penable,
    false.B
  )
  io.out.psel := Mux(
    state === sIDLE || state === sWAIT,
    io.in.psel,
    false.B
  )
  io.in.pready := state === sRESP

}

class APBDelayerWrapper(implicit p: Parameters) extends LazyModule {
  val node = APBIdentityNode()

  lazy val module = new Impl
  class Impl extends LazyModuleImp(this) {
    (node.in zip node.out) foreach { case ((in, edgeIn), (out, edgeOut)) =>
      // val delayer = Module(new APBDelayerChisel)
      val delayer = Module(new apb_delayer)

      delayer.io.clock := clock
      delayer.io.reset := reset
      delayer.io.in <> in
      out <> delayer.io.out
    }
  }
}

object APBDelayer {
  def apply()(implicit p: Parameters): APBNode = {
    val apbdelay = LazyModule(new APBDelayerWrapper)
    apbdelay.node
  }
}
