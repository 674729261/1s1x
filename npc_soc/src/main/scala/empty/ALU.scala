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
    // val funct3 = Input(UInt(3.W))
    // val is_sub_sra = Input(Bool())
    // val is_force_add = Input(Bool())
    val controls = Input(new ALUControl())
    val out = Output(UInt(WIDTH.W))

  })

  val adder = Module(new Adder(WIDTH))
  val log_width = log2Ceil(WIDTH)

  adder.io.A := io.A
  adder.io.B := io.B
  adder.io.Cin := io.controls.is_alu_b_inv
  io.out := Mux1H(
    Seq(
      (io.controls.is_alu_add || io.controls.is_alu_sub) -> adder.io.out,
      io.controls.is_alu_slt -> Cat(
        0.U((WIDTH - 1).W),
        adder.io.out(WIDTH - 1) ^ adder.io.overflow
      ),
      io.controls.is_alu_sltu -> Cat(0.U((WIDTH - 1).W), ~adder.io.Cout),
      io.controls.is_alu_sll -> (io.A << io.B(log_width - 1, 0)),
      io.controls.is_alu_sra -> (io.A.asSInt >> io.B(log_width - 1, 0)).asUInt,
      io.controls.is_alu_srl -> (io.A >> io.B(log_width - 1, 0)),
      io.controls.is_alu_and -> (io.A & io.B),
      io.controls.is_alu_or -> (io.A | io.B),
      io.controls.is_alu_xor -> (io.A ^ io.B)
    )
  )
}
