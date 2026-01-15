package ysyx

import chisel3._
import chisel3.util._
import chisel3.experimental.Analog

import freechips.rocketchip.amba.axi4._
import freechips.rocketchip.amba.apb._
import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._

class SDRAMIO extends Bundle {
  val clk = Output(Bool())
  val cke = Output(Bool())
  val cs = Output(Bool())
  val ras = Output(Bool())
  val cas = Output(Bool())
  val we = Output(Bool())
  val a = Output(UInt(13.W))
  val ba = Output(UInt(2.W))
  val dqm0_low = Output(UInt(2.W))
  val dq0_low = Analog(16.W)
  val dqm0_high = Output(UInt(2.W))
  val dq0_high = Analog(16.W)

  val dqm1_low = Output(UInt(2.W))
  val dq1_low = Analog(16.W)
  val dqm1_high = Output(UInt(2.W))
  val dq1_high = Analog(16.W)
}

class SDRAMIO_SINGLE extends Bundle {
  val clk = Output(Bool())
  val cke = Output(Bool())
  val cs = Output(Bool())
  val ras = Output(Bool())
  val cas = Output(Bool())
  val we = Output(Bool())
  val a = Output(UInt(13.W))
  val ba = Output(UInt(2.W))
  val dqm = Output(UInt(2.W))
  val dq = Analog(16.W)
}

class sdram_top_axi extends BlackBox {
  val io = IO(new Bundle {
    val clock = Input(Clock())
    val reset = Input(Bool())
    val in = Flipped(
      new AXI4Bundle(
        AXI4BundleParameters(addrBits = 32, dataBits = 32, idBits = 4)
      )
    )
    val sdram = new SDRAMIO
  })
}

class sdram_top_apb extends BlackBox {
  val io = IO(new Bundle {
    val clock = Input(Clock())
    val reset = Input(Bool())
    val in =
      Flipped(new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32)))
    val sdram = new SDRAMIO
  })
}

class sdram extends BlackBox {
  val io = IO(Flipped(new SDRAMIO))
}

class sdramChisel extends RawModule {
  val io = IO(Flipped(new SDRAMIO_SINGLE))
  val sout = Wire(UInt(16.W))
  val sen = Wire(Bool())
  val di = TriStateInBuf(io.dq, sout, sen)
  withClockAndReset(io.clk.asClock, !io.cke) {
    val mem = Mem(4 * 8192 * 512, Vec(2, UInt(8.W)))
    val mode_reg = RegInit(0.U(13.W))

    val sBANK_IDLE :: sBANK_ACTIVATE :: Nil = Enum(2)
    val activated_row_bank = Reg(Vec(4, UInt(13.W)))
    val state_bank = RegInit(VecInit(Seq.fill(4)(sBANK_IDLE)))

    val sIDLE :: sREAD_WAIT :: sBURST_READ :: sBURST_WRITE :: Nil = Enum(4)
    val state = RegInit(sIDLE)

    val cmd_nop =
      (io.cs) || (io.ras && io.cas && io.we)
    val cmd_active = !io.cs && !io.ras && io.cas && io.we
    val cmd_read = !io.cs && io.ras && !io.cas && io.we
    val cmd_write = !io.cs && io.ras && !io.cas && !io.we
    val cmd_burst_terminate = !io.cs && io.ras && io.cas && !io.we
    val cmd_mode = !io.cs && !io.ras && !io.cas && !io.we
    val cl_counter = Reg(UInt(3.W))
    val burst_counter = Reg(UInt(4.W))

    val cas_latency = mode_reg(6, 4)
    val burst_length = mode_reg(2, 0)
    val burst_end = (1.U(4.W) << Cat(0.U(1.W), burst_length))
    state := MuxLookup(state, sIDLE)(
      Seq(
        sIDLE -> MuxCase(
          state,
          Seq(
            // cmd_read -> Mux(cas_latency === 1.U(3.W), sBURST_READ, sREAD_WAIT),
            cmd_read -> sREAD_WAIT,
            cmd_write -> Mux(burst_length === 0.U, sIDLE, sBURST_WRITE)
          )
        ),
        sREAD_WAIT -> Mux(
          cl_counter === cas_latency - 1.U,
          sBURST_READ,
          sREAD_WAIT
        ),
        sBURST_READ -> Mux(
          burst_counter === burst_end - 1.U(4.W) || cmd_burst_terminate,
          sIDLE,
          sBURST_READ
        ),
        sBURST_WRITE -> Mux(
          burst_counter === burst_end - 2.U(4.W) || cmd_burst_terminate,
          sIDLE,
          sBURST_WRITE
        )
      )
    )
    val block_id = RegEnable(io.ba, state === sIDLE && (cmd_read || cmd_write))
    val addr_counter = Reg(UInt(9.W))

    for (i <- 0 until 4) {
      when(state === sIDLE && cmd_active && (io.ba === i.U)) {
        activated_row_bank(i) := io.a(12, 0)
        state_bank(i) := sBANK_ACTIVATE
      }
    }

    mode_reg := Mux(state === sIDLE && cmd_mode, io.a(12, 0), mode_reg)

    cl_counter := MuxCase(
      0.U,
      Seq(
        (state === sREAD_WAIT) -> (cl_counter + 1.U),
        (state === sIDLE) -> 0.U
      )
    )

    burst_counter := MuxCase(
      0.U,
      Seq(
        (state === sBURST_READ) -> (burst_counter + 1.U),
        (state === sBURST_WRITE) -> (burst_counter + 1.U),
        (state === sIDLE) -> 0.U
      )
    )

    addr_counter := MuxCase(
      addr_counter,
      Seq(
        (state === sIDLE && cmd_read) -> io.a(8, 0),
        (state === sIDLE && cmd_write) -> (io.a(8, 0) + 1.U(9.W)),
        (state === sBURST_READ) -> (addr_counter + 1.U),
        (state === sBURST_WRITE) -> (addr_counter + 1.U)
      )
    )
    val pointer_r =
      Cat(
        block_id,
        activated_row_bank(block_id),
        addr_counter
      )
    val pointer_w =
      Cat(
        Mux(state === sIDLE, io.ba, block_id),
        activated_row_bank(Mux(state === sIDLE, io.ba, block_id)),
        Mux(state === sIDLE, io.a(8, 0), addr_counter)
      )
    sen := state === sBURST_READ
    sout := mem.read(pointer_r).asUInt

    when(state === sBURST_WRITE || (state === sIDLE && cmd_write)) {
      // val mask64 = Cat(Fill(8, ~io.dqm(1)), Fill(8, ~io.dqm(0)))
      // val new_data = (mem.read(pointer_w) & ~mask64) | (di.asUInt & mask64)
      val vec_din = di.asTypeOf(Vec(2, UInt(8.W)))
      mem.write(pointer_w, vec_din, (~io.dqm).asBools)
    }

    when(state =/= sIDLE) {
      assert(
        cmd_nop || cmd_burst_terminate,
        "Invalid command during active operation"
      )
    }

    when(state === sIDLE && cmd_read) {
      assert(
        state_bank(io.ba) === sBANK_ACTIVATE,
        "Read command issued to precharged bank"
      )
      assert(
        cas_latency >= 1.U && cas_latency <= 3.U,
        "Unsupported CAS latency"
      )
      assert(
        burst_length < 4.U,
        "Unsupported burst length for read operation"
      )
    }
    when(state === sIDLE && cmd_write) {
      assert(
        burst_length < 4.U,
        "Unsupported burst length for write operation"
      )
    }

  }

}

class AXI4SDRAM(address: Seq[AddressSet])(implicit p: Parameters)
    extends LazyModule {
  val beatBytes = 4
  val node = AXI4SlaveNode(
    Seq(
      AXI4SlavePortParameters(
        Seq(
          AXI4SlaveParameters(
            address = address,
            executable = true,
            supportsWrite = TransferSizes(1, beatBytes),
            supportsRead = TransferSizes(1, beatBytes),
            interleavedId = Some(0)
          )
        ),
        beatBytes = beatBytes
      )
    )
  )

  lazy val module = new Impl
  class Impl extends LazyModuleImp(this) {
    val (in, _) = node.in(0)
    val sdram_bundle = IO(new SDRAMIO)

    val msdram = Module(new sdram_top_axi)
    msdram.io.clock := clock
    msdram.io.reset := reset.asBool
    msdram.io.in <> in
    sdram_bundle <> msdram.io.sdram
  }
}

class APBSDRAM(address: Seq[AddressSet])(implicit p: Parameters)
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
    val sdram_bundle = IO(new SDRAMIO)

    val msdram = Module(new sdram_top_apb)
    msdram.io.clock := clock
    msdram.io.reset := reset.asBool
    msdram.io.in <> in
    sdram_bundle <> msdram.io.sdram
  }
}
