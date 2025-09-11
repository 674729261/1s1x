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

  val is_lt = Wire(Bool())
  val is_eq = Wire(Bool())
  is_lt := (io.A(31) & ~io.B(31)) | // - < +
    ((~(io.A(31) ^ io.B(31))) & adder.io.out(31)) // A - B < 0
  is_eq := adder.io.out === 0.U(32.W)

  val is_unsigned = io.funct3(1)
  val compared = Mux(is_unsigned, ~adder.io.Cout, is_lt)

  io.jump := MuxLookup(Cat(io.funct3(2), io.funct3(0)), false.B)(
    Seq(
      "b00".U(3.W) -> is_eq,
      "b01".U(3.W) -> ~is_eq,
      "b10".U(3.W) -> compared,
      "b11".U(3.W) -> ~compared
    )
  )

}
