package empty

import chisel3._
import chisel3.util._

class GPR(CNT: Int) extends Module {
  val log_cnt = log2Ceil(CNT)
  val io = IO(new Bundle {
    val raddr1 = Input(UInt(log_cnt.W))
    val raddr2 = Input(UInt(log_cnt.W))
    val waddr = Input(UInt(log_cnt.W))
    val wdata = Input(UInt(CNT.W))
    val wen = Input(Bool())

    val rdata1 = Output(UInt(CNT.W))
    val rdata2 = Output(UInt(CNT.W))
  })

  val register_bank = Wire(new Bundle {
    val regs = Vec(CNT - 1, UInt(CNT.W))
    val x0 = UInt(CNT.W)
  })
  val ENs = Wire(Vec(CNT, Bool()))

  for (i <- 1 until CNT) {
    register_bank
      .regs(i - 1) := RegEnable(io.wdata, io.wen && ENs(i))
  }
  register_bank.x0 := 0.U(CNT.W)

  val data = register_bank.asTypeOf(Vec(CNT, UInt(CNT.W)))

  io.rdata1 := data(io.raddr1)
  io.rdata2 := data(io.raddr2)

  ENs := VecInit(UIntToOH(io.waddr).asBools)

}
