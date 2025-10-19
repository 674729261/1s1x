package empty

import chisel3._
import chisel3.util._

class CSR(csrList: Array[Int]) extends Module {
  val io = IO(new Bundle {
    val csr = Input(UInt(12.W))
    val wen = Input(Bool())
    val wdata = Input(UInt(32.W))
    val rdata = Output(UInt(32.W))
  })
  val cnt_csr = csrList.length
  val csrRegs = Wire(Vec(cnt_csr, UInt(32.W)))
  val csrSelOH = Wire(Vec(cnt_csr, Bool()))

  var s = new Array[(Bool, UInt)](cnt_csr);
  for (i <- 0 until cnt_csr) {
    csrSelOH(i) := (io.csr === csrList(i).U(32.W))
    val nxt = Wire(UInt(32.W))
    val writing = Wire(Bool())
    writing := io.wen && csrSelOH(i)

    if (csrList(i) == 0xb00)
      nxt := Mux(writing, io.wdata, csrRegs(i) + 1.U(32.W))
    else
      nxt := io.wdata

    csrRegs(i) := RegEnable(nxt, writing)
    s(i) = (csrSelOH(i) -> csrRegs(i))
  }

  io.rdata := Mux1H(s)

}
