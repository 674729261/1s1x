package empty

import chisel3._
import chisel3.util._
class RamLoadData extends RawModule {
  val io = IO(new Bundle {
    val word = Input(UInt(32.W))
    val lower2bit = Input(UInt(2.W))
    val is_word = Input(Bool())
    val is_half = Input(Bool())
    val is_byte = Input(Bool())
    val is_signed = Input(Bool())

    val out = Output(UInt(32.W))
  })
  val hw_temp = Wire(UInt(16.W))
  val b_temp = Wire(UInt(8.W))
  hw_temp := io.word(15, 0)
  b_temp := io.word(8, 0)
  when(io.is_word) {
    io.out := io.word
  }.elsewhen(io.is_half) {
    switch(io.lower2bit) {
      is("b00".U(2.W)) { hw_temp := io.word(15, 0) }
      is("b01".U(2.W)) { hw_temp := io.word(23, 8) }
      is("b10".U(2.W)) { hw_temp := io.word(31, 16) }
    }
    when(io.is_signed) {
      io.out := Cat(Fill(16, hw_temp(15)), hw_temp)
    }.otherwise { io.out := Cat(0.U(16.W), hw_temp) }
  }.elsewhen(io.is_byte) {
    switch(io.lower2bit) {
      is("b00".U(2.W)) { b_temp := io.word(7, 0) }
      is("b01".U(2.W)) { b_temp := io.word(15, 8) }
      is("b10".U(2.W)) { b_temp := io.word(23, 16) }
      is("b11".U(2.W)) { b_temp := io.word(31, 24) }
    }
    when(io.is_signed) {
      io.out := Cat(Fill(24, b_temp(7)), b_temp)
    }.otherwise { io.out := Cat(0.U(24.W), b_temp) }
  }.otherwise {
    io.out := io.word
  }

}
