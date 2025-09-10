package empty

package empty
import chisel3._
import chisel3.util._

class Branch(WIDTH: Int) extends RawModule {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val funct3 = Input(UInt(3.W))
    val jump = Output(UInt(WIDTH.W))

  })

  val adder = Module(new Adder(32))
  adder.io.A := io.A
  adder.io.B := ~io.B
  adder.io.Cin := true.B

  io.jump := false.B
  val is_lt = Wire(Bool())
  val is_eq = Wire(Bool())
  is_lt := (io.A(31) & ~io.B(31)) | // - < +
    ((~(io.A(31) ^ io.B(31))) & adder.io.out(31)) // A - B < 0
  is_eq := adder.io.out === 0.U(32.W)
  switch(io.funct3) {
    is("b000".U(3.W)) { // beq
      io.jump := is_eq
    }
    is("b001".U(3.W)) { // bne
      io.jump := ~is_eq
    }
    is("b100".U(3.W)) { // blt
      io.jump := is_lt
    }
    is("b101".U(3.W)) { // bge
      io.jump := ~is_lt
    }
    is("b110".U(3.W)) { // bltu
      io.jump := ~adder.io.Cout
    }
    is("b111".U(3.W)) { // bgeu
      io.jump := adder.io.Cout
    }
  }

}
