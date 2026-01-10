package ysyx

import chisel3._
import chisel3.util._

import freechips.rocketchip.amba.apb._
import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._

class GPIOIO extends Bundle {
  val out = Output(UInt(16.W))
  val in = Input(UInt(16.W))
  val seg = Output(Vec(8, UInt(8.W)))
}

class GPIOCtrlIO extends Bundle {
  val clock = Input(Clock())
  val reset = Input(Reset())
  val in = Flipped(
    new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32))
  )
  val gpio = new GPIOIO

}

class gpio_top_apb extends BlackBox {
  val io = IO(new GPIOCtrlIO)
}

class gpioChisel extends Module {
  val io = IO(new GPIOCtrlIO)
  val apb = io.in
  val gpio = io.gpio

  apb.pready := apb.psel & apb.penable
  apb.pslverr := false.B

  val read_fire = apb.psel && apb.penable && apb.pready && !apb.pwrite
  val write_fire = apb.psel && apb.penable && apb.pready && apb.pwrite
  val gpio_out_reg = RegInit(Vec(2, UInt(8.W)), VecInit(Seq.fill(2)(0.U(8.W))))
  gpio.out := gpio_out_reg.asUInt
  for (i <- 0 until 2) {
    gpio_out_reg(i) := Mux(
      write_fire && apb.paddr(3, 2) === 0.U && apb.pstrb(i),
      apb.pwdata(i * 8 + 7, i * 8),
      gpio_out_reg(i)
    )
  }
  val gpio_seq_reg = RegInit(Vec(8, UInt(8.W)), VecInit(Seq.fill(8)(0.U(8.W))))
  gpio.seg := gpio_seq_reg
  for (i <- 0 until 4) {
    gpio_seq_reg(i) := Mux(
      write_fire && apb.paddr(3, 2) === 2.U && apb.pstrb(i),
      apb.pwdata(i * 8 + 7, i * 8),
      gpio_seq_reg(i)
    )
  }

  for (i <- 0 until 4) {
    gpio_seq_reg(i + 4) := Mux(
      write_fire && apb.paddr(3, 2) === 3.U && apb.pstrb(i),
      apb.pwdata(i * 8 + 7, i * 8),
      gpio_seq_reg(i + 4)
    )
  }

  apb.prdata := Mux(
    read_fire,
    MuxCase(
      0.U,
      Seq(
        (apb.paddr(3, 2) === 0.U) -> Cat(
          0.U(16.W),
          gpio_out_reg(1),
          gpio_out_reg(0)
        ),
        (apb.paddr(3, 2) === 1.U) -> Cat(0.U(16.W), gpio.in),
        (apb.paddr(3, 2) === 2.U) -> Cat(
          gpio_seq_reg(3),
          gpio_seq_reg(2),
          gpio_seq_reg(1),
          gpio_seq_reg(0)
        ),
        (apb.paddr(3, 2) === 3.U) -> Cat(
          gpio_seq_reg(7),
          gpio_seq_reg(6),
          gpio_seq_reg(5),
          gpio_seq_reg(4)
        )
      )
    ),
    0.U
  )

}

class APBGPIO(address: Seq[AddressSet])(implicit p: Parameters)
    extends LazyModule {
  val node = APBSlaveNode(
    Seq(
      APBSlavePortParameters(
        Seq(
          APBSlaveParameters(
            address = address,
            executable = true,
            supportsRead = true,
            supportsWrite = true
          )
        ),
        beatBytes = 4
      )
    )
  )

  lazy val module = new Impl
  class Impl extends LazyModuleImp(this) {
    val (in, _) = node.in(0)
    val gpio_bundle = IO(new GPIOIO)

    val mgpio = Module(new gpioChisel)
    mgpio.io.clock := clock
    mgpio.io.reset := reset
    mgpio.io.in <> in
    gpio_bundle <> mgpio.io.gpio
  }
}
