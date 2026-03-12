package empty
import chisel3._
import chisel3.util._
class Adder(WIDTH: Int) extends RawModule {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val Cin = Input(Bool())
    val out = Output(UInt(WIDTH.W))
    val Cout = Output(Bool())
    val overflow = Output(Bool())
  })
  val result33 = Wire(UInt((WIDTH + 1).W))
  // result33 := Cat(0.U(1.W), io.A) + Cat(0.U(1.W), io.B) + Cat(
  //   0.U(WIDTH.W),
  //   io.Cin
  // )
  result33 := io.A +& io.B + io.Cin
  io.out := result33((WIDTH - 1), 0)
  io.Cout := result33(WIDTH)

  val signA = io.A(WIDTH - 1)
  val signB = io.B(WIDTH - 1)
  val signS = result33(WIDTH - 1)

  io.overflow := (signA === signB) && (signA =/= signS)
}

class ALU(WIDTH: Int) extends RawModule {
  val io = IO(new Bundle {
    val A = Input(UInt(WIDTH.W))
    val B = Input(UInt(WIDTH.W))
    val funct3 = Input(UInt(3.W))
    val is_sub_sra = Input(Bool())
    val is_force_add = Input(Bool())
    val out = Output(UInt(WIDTH.W))

  })

  val adder = Module(new Adder(WIDTH))

  // add
  adder.io.A := io.A
  adder.io.B := io.B
  adder.io.Cin := false.B
  io.out := adder.io.out

  // var log_width = 0;
  // var upper = 1;
  // while (upper < WIDTH) {
  //   upper *= 2
  //   log_width += 1
  // }
  val log_width = log2Ceil(WIDTH)

  when(~io.is_force_add) {
    switch(io.funct3) {
      is("b000".U(3.W)) { // add / sub / slt
        when(io.is_sub_sra) { // sub
          adder.io.B := ~io.B
          adder.io.Cin := true.B
        }.otherwise {}
      }
      is("b010".U(3.W)) { // slt
        adder.io.B := ~io.B
        adder.io.Cin := true.B
        // io.out := Cat(
        //   0.U(31.W),
        //   (io.A(WIDTH - 1) & ~io.B(WIDTH - 1)) | // - < +
        //     ((~(io.A(WIDTH - 1) ^ io.B(WIDTH - 1))) & adder.io.out(
        //       WIDTH - 1
        //     )) // A - B < 0
        //   // io.A.asSInt < io.B.asSInt
        // )
        io.out := Cat(
          0.U((WIDTH - 1).W),
          adder.io.out(WIDTH - 1) ^ adder.io.overflow
        )
      }
      is("b011".U(3.W)) { // sltu
        adder.io.B := ~io.B
        adder.io.Cin := true.B
        io.out := Cat(0.U((WIDTH - 1).W), ~adder.io.Cout)
      }

      is("b001".U(3.W)) { // sll
        io.out := io.A << io.B(log_width - 1, 0)
      }
      is("b101".U(3.W)) { // srl / sra
        when(io.is_sub_sra) { // sra
          io.out := (io.A.asSInt >> io.B(log_width - 1, 0)).asUInt
        }.otherwise { // srl
          io.out := io.A >> io.B(log_width - 1, 0)
        }
      }

      is("b111".U(3.W)) { // and
        io.out := io.A & io.B
      }
      is("b110".U(3.W)) { // or
        io.out := io.A | io.B
      }
      is("b100".U(3.W)) { // xor
        io.out := io.A ^ io.B
      }
    }
  }
}
