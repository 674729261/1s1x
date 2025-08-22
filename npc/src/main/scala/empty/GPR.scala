package empty

import chisel3._
import chisel3.util._

class GPR extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val raddr1 = Input(UInt(5.W))
    val raddr2 = Input(UInt(5.W))
    val waddr = Input(UInt(5.W))
    val wdata = Input(UInt(32.W))
    val wen = Input(Bool())

    val rdata1 = Output(UInt(32.W))
    val rdata2 = Output(UInt(32.W))
  })

  val register_bank = Wire(new Bundle {
    val regs = Vec(31, UInt(32.W))
    val x0 = UInt(32.W)
  })
  val ENs = Wire(Vec(32, Bool()))

  for (i <- 1 until 32) {
    register_bank
      .regs(i - 1) := RegEnable(io.wdata, 0.U(32.W), io.wen && ENs(i))
  }
  register_bank.x0 := 0.U(32.W)

  val data = register_bank.asTypeOf(Vec(32, UInt(32.W)))

  io.rdata1 := data(io.raddr1)
  io.rdata2 := data(io.raddr2)

  ENs := VecInit(UIntToOH(io.waddr).asBools)

}
