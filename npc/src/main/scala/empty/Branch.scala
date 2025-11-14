package empty

package empty
import chisel3._
import chisel3.util._

class Branch(WIDTH: Int) extends RawModule {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val funct3 = Input(UInt(3.W))
    val jump = Output(Bool())

  })

  val adder = Module(new Adder(32))
  adder.io.A := io.A
  adder.io.B := ~io.B
  adder.io.Cin := true.B
  adder.io.overflow := DontCare

  val is_lt = Wire(Bool())
  val is_eq = Wire(Bool())
  is_lt := Cat(
    0.U((WIDTH - 1).W),
    adder.io.out(WIDTH - 1) ^ adder.io.overflow
  )
  // is_lt := (io.A(31) & ~io.B(31)) | // - < +
  //   ((~(io.A(31) ^ io.B(31))) & adder.io.out(31)) // A - B < 0
  is_eq := adder.io.out === 0.U(32.W)

  val is_unsigned = io.funct3(1)
  val compared = Mux(is_unsigned, ~adder.io.Cout, is_lt)

  io.jump := io.funct3(0) ^ Mux(io.funct3(2), compared, is_eq)
}
