package ysyx

import chisel3._
import chisel3.util._

class bitrev extends BlackBox {
  val io = IO(Flipped(new SPIIO(1)))
}

class bitrevChisel extends Module {
  val io = IO(Flipped(new SPIIO(1)))
  val nxt_byte = Wire(UInt(8.W))
  val do_shift_recv = Wire(Bool())
  val do_shift_tran = Wire(Bool())

  val byte_regs = RegNext(nxt_byte)

  val sck_r = RegNext(io.sck)
  val neg_edge = !io.sck && sck_r
  val pos_edge = io.sck && !sck_r

  val sIDLE :: sRECV :: sTRAN :: sEND :: Nil = Enum(4)
  val state = RegInit(sIDLE)
  val counter = Reg(UInt(4.W))

  counter := MuxCase(
    counter,
    Seq(
      (state === sIDLE && io.ss === 0.U(1.W)) -> 0.U(4.W),
      (state === sRECV && neg_edge) -> (counter + 1.U(4.W)),
      (state === sTRAN && pos_edge) -> (counter - 1.U(4.W))
    )
  )

  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(!io.ss, sRECV, sIDLE),
      sRECV -> Mux(neg_edge && counter === 7.U(4.W), sTRAN, sRECV),
      sTRAN -> Mux(pos_edge && counter === 1.U(4.W), sEND, sTRAN),
      sEND -> Mux(io.ss === 1.U(1.W), sIDLE, sEND)
    )
  )

  do_shift_recv := (state === sRECV && neg_edge)
  do_shift_tran := (state === sTRAN && pos_edge)

  nxt_byte := MuxCase(
    byte_regs,
    Seq(
      do_shift_recv -> Cat(byte_regs(6, 0), io.mosi),
      do_shift_tran -> Cat(0.U(1.W), byte_regs(7, 1))
    )
  )
  val out_r = RegEnable(byte_regs(0), do_shift_tran)
  io.miso := out_r | io.ss
}
