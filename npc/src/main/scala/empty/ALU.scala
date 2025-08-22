package empty
import chisel3._
import chisel3.util._
class Adder(WIDTH: Int) extends RawModule {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val out = Output(UInt(WIDTH.W))
  })

  io.out := io.A + io.B
}
