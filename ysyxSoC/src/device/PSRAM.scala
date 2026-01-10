package ysyx

import chisel3._
import chisel3.util._
import chisel3.experimental.Analog

import freechips.rocketchip.amba.apb._
import org.chipsalliance.cde.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.util._
import chisel3.assert.Assert

class QSPIIO extends Bundle {
  val sck = Output(Bool())
  val ce_n = Output(Bool())
  val dio = Analog(4.W)
}

class psram_top_apb extends BlackBox {
  val io = IO(new Bundle {
    val clock = Input(Clock())
    val reset = Input(Reset())
    val in =
      Flipped(new APBBundle(APBBundleParameters(addrBits = 32, dataBits = 32)))
    val qspi = new QSPIIO
  })
}

class psram extends BlackBox {
  val io = IO(Flipped(new QSPIIO))
}

class psramChisel extends Module {
  val io = IO(Flipped(new QSPIIO))

  val sout = Wire(UInt(4.W))
  val sen = Wire(Bool())

  val di = TriStateInBuf(io.dio, sout, sen)

  val qpi_mode = RegInit(false.B)

  val mem = Mem(1024 * 1024 * 4, UInt(8.W))

  val bitcnt = RegInit(0.U(8.W))

  val sck_r = RegNext(io.sck)
  val sck_falling = sck_r & ~io.sck
  val sck_rising = ~sck_r & io.sck
  val cmd_nxt = Wire(UInt(8.W))
  val cmd = RegNext(cmd_nxt)
  val sIDLE :: sCMD :: sADDR :: sWAIT :: sDATA :: sWRITE :: sSLEEP :: Nil =
    Enum(7)
  val state = RegInit(sIDLE)

  state := Mux(
    io.ce_n,
    sIDLE,
    MuxLookup(state, sIDLE)(
      Seq(
        sIDLE -> sCMD,
        sCMD -> Mux(
          sck_rising && bitcnt === Mux(qpi_mode, 1.U(8.W), 7.U(8.W)),
          Mux(cmd_nxt === 0x35.U(8.W), sSLEEP, sADDR),
          sCMD
        ),
        sADDR -> Mux(
          sck_rising && bitcnt === 5.U(8.W),
          Mux(cmd === 0x38.U(8.W), sWRITE, sWAIT),
          sADDR
        ),
        sWAIT -> Mux(sck_rising && bitcnt === 5.U(8.W), sDATA, sWAIT),
        sDATA -> sDATA,
        sWRITE -> sWRITE,
        sSLEEP -> sSLEEP
      )
    )
  )

  qpi_mode := qpi_mode || (state === sCMD && sck_rising && bitcnt === Mux(
    qpi_mode,
    1.U(8.W),
    7.U(8.W)
  ) && cmd_nxt === 0x35.U(8.W))

  val addr = Reg(UInt(24.W))
  val wdata = Reg(UInt(4.W))

  sen := state === sDATA
  cmd_nxt := Mux(
    state === sCMD && sck_rising,
    Mux(qpi_mode, Cat(cmd(3, 0), di), Cat(cmd(6, 0), di(0))),
    cmd
  )
  addr := MuxCase(
    addr,
    Seq(
      (state === sADDR && sck_rising) -> Cat(addr(19, 0), di),
      (state === sDATA && sck_rising && bitcnt === 1.U) -> (addr + 1.U(24.W)),
      (state === sWRITE && sck_rising && bitcnt === 1.U) -> (addr + 1.U(24.W))
    )
  )

  wdata := MuxCase(
    wdata,
    Seq((state === sWRITE && sck_rising && bitcnt === 0.U) -> di)
  )
  val cur_byte = mem.read(addr)

  sout := Mux(bitcnt === 0.U(8.W), cur_byte(7, 4), cur_byte(3, 0))

  bitcnt := MuxCase(
    bitcnt,
    Seq(
      (state === sIDLE) -> 0.U,
      (state === sCMD && sck_rising) -> Mux(
        bitcnt === Mux(qpi_mode, 1.U(8.W), 7.U(8.W)),
        0.U,
        bitcnt + 1.U
      ),
      (state === sADDR && sck_rising) -> Mux(bitcnt === 5.U, 0.U, bitcnt + 1.U),
      (state === sWAIT && sck_rising) -> Mux(bitcnt === 5.U, 0.U, bitcnt + 1.U),
      (state === sDATA && sck_rising) -> Mux(bitcnt === 1.U, 0.U, 1.U),
      (state === sWRITE && sck_rising) -> Mux(bitcnt === 1.U, 0.U, 1.U)
    )
  )

  when(state === sWRITE && sck_falling && bitcnt === 1.U) {
    mem.write(addr, Cat(wdata, di))
  }

  when(state === sADDR) {
    assert(
      cmd === 0xeb.U(8.W) || cmd === 0x38.U(8.W),
      "PSRAM command must be 0x35, 0xeb or 0x38"
    )
  }

}

class APBPSRAM(address: Seq[AddressSet])(implicit p: Parameters)
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
    val qspi_bundle = IO(new QSPIIO)

    val mpsram = Module(new psram_top_apb)
    mpsram.io.clock := clock
    mpsram.io.reset := reset
    mpsram.io.in <> in
    qspi_bundle <> mpsram.io.qspi
  }
}
