package empty

import chisel3._
import chisel3.util._
import chisel3.util.circt.Mux4Cell
class RamLoadData extends RawModule {
  val io = IO(new Bundle {
    val word = Input(UInt(32.W))
    val lower2bit = Input(UInt(2.W))
    val is_word = Input(Bool())
    val is_half = Input(Bool())
    val is_byte = Input(Bool())
    val is_unsigned = Input(Bool())

    val out = Output(UInt(32.W))
  })

  val w_out = Wire(UInt(32.W))
  val short = Wire(UInt(16.W))
  val h = Wire(UInt(32.W))
  val hu = Wire(UInt(32.W))
  val h_out = Wire(UInt(32.W))
  val byte = Wire(UInt(8.W))
  val b = Wire(UInt(32.W))
  val bu = Wire(UInt(32.W))
  val b_out = Wire(UInt(32.W))

  w_out := io.word
  short := Mux(io.lower2bit(1), io.word(31, 16), io.word(15, 0))
  hu := Cat(0.U(16.W), short)
  h := Cat(Fill(16, short(15)), short)
  h_out := Mux(io.is_unsigned, hu, h)

  byte :=
    Mux4Cell(
      io.lower2bit,
      v3 = io.word(31, 24),
      v2 = io.word(23, 16),
      v1 = io.word(15, 8),
      v0 = io.word(7, 0)
    )
  // MuxLookup(io.lower2bit, 0.U(8.W))(
  //   Seq(
  //     "b00".U(2.W) -> io.word(7, 0),
  //     "b01".U(2.W) -> io.word(15, 8),
  //     "b10".U(2.W) -> io.word(23, 16),
  //     "b11".U(2.W) -> io.word(31, 24)
  //   )
  // )
  bu := Cat(0.U(24.W), byte)
  b := Cat(Fill(24, byte(7)), byte)
  b_out := Mux(io.is_unsigned, bu, b)
  io.out := Mux1H(
    Seq(io.is_byte -> b_out, io.is_half -> h_out, io.is_word -> w_out)
  )

}
