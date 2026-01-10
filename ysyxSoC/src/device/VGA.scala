package ysyx

import chisel3._
import chisel3.util._

import freechips.rocketchip.amba.apb._
import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._
import chisel3.SramTarget

class VGAIO extends Bundle {
  val r = Output(UInt(8.W))
  val g = Output(UInt(8.W))
  val b = Output(UInt(8.W))
  val hsync = Output(Bool())
  val vsync = Output(Bool())
  val valid = Output(Bool())
}

class VGACtrlIO extends Bundle {
  val clock = Input(Clock())
  val reset = Input(Bool())
  val in = Flipped(
    new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32))
  )
  val vga = new VGAIO
}

class vga_top_apb extends BlackBox {
  val io = IO(new VGACtrlIO)
}

class vgaChisel extends Module {
  val io = IO(new VGACtrlIO)
  val apb = io.in
  val vga = io.vga
  apb.pslverr := false.B
  apb.pready := apb.psel && apb.penable
  val fire = apb.psel && apb.penable && apb.pready
  val buffer = SyncReadMem(640 * 480, Vec(4, UInt(8.W)))

  val do_read = apb.psel && !apb.penable && !apb.pwrite
  val buffer_dout = Wire(UInt(32.W))
  buffer_dout := buffer.read(apb.paddr(19, 2), do_read).asUInt
  apb.prdata := Mux(fire && !apb.pwrite, buffer_dout, 0.U)

  val do_write = apb.psel && apb.penable && apb.pwrite
  when(do_write) {
    buffer.write(
      apb.paddr(19, 2),
      apb.pwdata.asTypeOf(Vec(4, UInt(8.W))),
      apb.pstrb.asBools
    )
  }

  val counter_x = RegInit(0.U(10.W))
  val counter_y = RegInit(0.U(10.W))
  counter_x := Mux(counter_x === 799.U, 0.U, counter_x + 1.U)
  counter_y := Mux(
    counter_x === 799.U,
    Mux(counter_y === 524.U, 0.U, counter_y + 1.U),
    counter_y
  )
  val h_frontporch = 96.U(10.W)
  val h_active = 144.U(10.W)
  val h_backporch = 784.U(10.W)
  val h_total = 800.U(10.W)

  val v_frontporch = 2.U(10.W)
  val v_active = 35.U(10.W)
  val v_backporch = 515.U(10.W)
  val v_total = 525.U(10.W)

  val full_pixel = (640 * 480 - 1).U

  val x_valid = counter_x > h_active && counter_x <= h_backporch
  val y_valid = counter_y > v_active && counter_y <= v_backporch

  val pixel_valid = x_valid && y_valid

  val pointer = RegInit(0.U(20.W))
  pointer := Mux(
    pixel_valid,
    Mux(pointer === full_pixel, 0.U, pointer + 1.U),
    pointer
  )
  val pixel = buffer.read(pointer, pixel_valid).asUInt

  val hs = counter_x >= h_frontporch
  val vs = counter_y >= v_frontporch

  vga.valid := RegNext(pixel_valid, false.B)
  vga.vsync := RegNext(vs, false.B)
  vga.hsync := RegNext(hs, false.B)
  vga.r := pixel(7, 0)
  vga.g := pixel(15, 8)
  vga.b := pixel(23, 16)
}

class APBVGA(address: Seq[AddressSet])(implicit p: Parameters)
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
    val vga_bundle = IO(new VGAIO)

    val mvga = Module(new vgaChisel)
    mvga.io.clock := clock
    mvga.io.reset := reset
    mvga.io.in <> in
    vga_bundle <> mvga.io.vga
  }
}
