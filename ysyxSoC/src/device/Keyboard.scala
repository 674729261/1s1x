package ysyx

import chisel3._
import chisel3.util._

import freechips.rocketchip.amba.apb._
import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._

class PS2IO extends Bundle {
  val clk = Input(Bool())
  val data = Input(Bool())
}

class PS2CtrlIO extends Bundle {
  val clock = Input(Clock())
  val reset = Input(Bool())
  val in = Flipped(
    new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32))
  )
  val ps2 = new PS2IO
}

class ps2_top_apb extends BlackBox {
  val io = IO(new PS2CtrlIO)
}

class ps2Chisel(fifo_addr_width: Int = 5) extends Module {
  val io = IO(new PS2CtrlIO)
  val fifo = Mem(1 << fifo_addr_width, UInt(8.W))
  val pointer_read = RegInit(0.U((fifo_addr_width + 1).W))
  val pointer_write = RegInit(0.U((fifo_addr_width + 1).W))
  val ps_clk_r = RegNext(io.ps2.clk)
  val ps_data_r = RegNext(io.ps2.data)
  val negedge_clk = ps_clk_r && !io.ps2.clk

  val sIDLE :: sREAD :: sEND :: Nil = Enum(3)
  val state = RegInit(sIDLE)
  val counter = Reg(UInt(4.W))
  val fire = state === sREAD && negedge_clk

  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(negedge_clk && !io.ps2.data, sREAD, sIDLE),
      sREAD -> Mux(counter === 8.U && fire, sEND, sREAD),
      sEND -> sIDLE
    )
  )

  counter := MuxCase(
    counter,
    Seq(
      (state === sIDLE) -> 0.U,
      fire -> (counter + 1.U)
    )
  )
  val code_buffer = Reg(UInt(8.W))
  val parity_sum = Reg(Bool())
  parity_sum := MuxCase(
    parity_sum,
    Seq(
      (state === sIDLE) -> 0.U,
      fire -> (parity_sum ^ io.ps2.data)
    )
  )
  code_buffer := Mux(
    fire && counter =/= 8.U,
    Cat(io.ps2.data, code_buffer(7, 1)),
    code_buffer
  )

  val empty = pointer_write === pointer_read
  val full = pointer_write(fifo_addr_width - 1, 0) === pointer_read(
    fifo_addr_width - 1,
    0
  ) && pointer_write(
    fifo_addr_width
  ) =/= pointer_read(fifo_addr_width)

  when(state === sEND) {
    fifo.write(pointer_write(fifo_addr_width - 1, 0), code_buffer)
    pointer_write := pointer_write + 1.U
  }
  io.in.pready := io.in.penable && io.in.psel
  io.in.pslverr := false.B
  val apb_fire = io.in.penable && io.in.psel && io.in.pready
  io.in.prdata := Mux(
    apb_fire,
    Mux(
      !empty,
      Cat(0.U(24.W), fifo.read(pointer_read(fifo_addr_width - 1, 0))),
      0.U
    ),
    0.U
  )

  when(apb_fire) {
    pointer_read := Mux(!empty, pointer_read + 1.U, pointer_read)
  }

  when(state === sEND) {
    // assert(!full, "Scancode FIFO is full")
    assert(parity_sum, "PS/2 parity check failed")
  }
}

class APBKeyboard(address: Seq[AddressSet])(implicit p: Parameters)
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
    val ps2_bundle = IO(new PS2IO)

    val mps2 = Module(new ps2Chisel)
    mps2.io.clock := clock
    mps2.io.reset := reset
    mps2.io.in <> in
    ps2_bundle <> mps2.io.ps2
  }
}
