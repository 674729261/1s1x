package empty

import chisel3._
import chisel3.util._
import chisel3.util.circt.Mux4Cell

class RamWriteData extends RawModule {
  val io = IO(new Bundle {
    val word = Input(UInt(32.W))
    val lower2bit = Input(UInt(2.W))
    val is_word = Input(Bool())
    val is_half = Input(Bool())
    val is_byte = Input(Bool())

    val out = Output(UInt(32.W))
    val mask = Output(UInt(4.W))
  })

  val out_t = Wire(Vec(4, UInt(8.W)))
  out_t(3) := Mux1H(
    Seq(
      io.is_word -> io.word(31, 24),
      io.is_half -> Mux(
        io.lower2bit(1),
        io.word(15, 8),
        io.word(31, 24)
      ),
      io.is_byte -> Mux(
        io.lower2bit === "b11".U(2.W),
        io.word(7, 0),
        io.word(31, 24)
      )
    )
  )
  out_t(2) := Mux1H(
    Seq(
      io.is_word -> io.word(23, 16),
      (io.is_half || io.is_byte) -> Mux(
        io.lower2bit(1),
        io.word(7, 0),
        io.word(23, 16)
      )
      // io.is_byte -> Mux(
      //   io.lower2bit === "b10".U(2.W),
      //   io.word(7, 0),
      //   io.word(23, 16)
      // )
    )
  )

  out_t(1) := Mux1H(
    Seq(
      (io.is_word || io.is_half) -> io.word(15, 8),
      // io.is_half -> io.word(15, 8),
      io.is_byte -> Mux(
        io.lower2bit(0),
        io.word(7, 0),
        io.word(15, 8)
      )
    )
  )

  out_t(0) := io.word(7, 0)

  // val out_t = Wire(UInt(32.W))
  // out_t := io.word << Cat(io.lower2bit, 0.U(3.W))

  val mask_base = Mux1H(
    Seq(
      io.is_word -> "b1111".U(4.W),
      io.is_half -> Mux(io.lower2bit(1), "b1100".U(4.W), "b0011".U(4.W)),
      io.is_byte ->
        Mux4Cell(
          io.lower2bit,
          v3 = "b1000".U(4.W),
          v2 = "b0100".U(4.W),
          v1 = "b0010".U(4.W),
          v0 = "b0001".U(4.W)
        )
      // MuxLookup (io.lower2bit, "b1111".U(4.W))(
      //   Seq(
      //     "b00".U(2.W) -> "b0001".U(4.W),
      //     "b01".U(2.W) -> "b0010".U(4.W),
      //     "b10".U(2.W) -> "b0100".U(4.W),
      //     "b11".U(2.W) -> "b1000".U(4.W)
      //   )
      // )
    )
  )

  // val mask_base = Mux1H(
  //   Seq(
  //     io.is_word -> "b1111".U(4.W),
  //     io.is_half -> "b0011".U(4.W),
  //     io.is_byte -> "b0001".U(4.W)
  //   )
  // )

  io.mask := mask_base

  io.out := out_t.asUInt
  // io.out := Cat(out_t(3), out_t(2), out_t(1), out_t(0))

}
