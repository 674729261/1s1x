package empty

import chisel3._
import chisel3.util._

class CSR extends Module {
  val io = IO(new Bundle {
    val csr = Input(UInt(12.W))
    val wen = Input(Bool())
    val wdata = Input(UInt(32.W))
    val rdata = Output(UInt(32.W))

    val mepc = Output(UInt(32.W))
    val mtvec = Output(UInt(32.W))

    val cur_pc = Input(UInt(32.W))
    val new_cause = Input(UInt(32.W))
    val interruption = Input(Bool())
    val ok_to_step = Input(Bool())
  })

  val is_mvendorid = io.csr === 0xf11.U(12.W)
  val is_marchid = io.csr === 0xf12.U(12.W)
  val is_mcycle = io.csr === 0xb00.U(12.W)
  val is_mstatus = io.csr === 0x300.U(12.W)
  val is_mcause = io.csr === 0x342.U(12.W)
  val is_mtvec = io.csr === 0x305.U(12.W)
  val is_mepc = io.csr === 0x341.U(12.W)

  val csr_mvendorid = 0x79737978.U(32.W)
  val csr_marchid = 0x17eb198.U(32.W)

  val mcycle_nxt = Wire(UInt(32.W))
  val csr_mcycle = RegNext(mcycle_nxt, 0.U(32.W))
  mcycle_nxt := Mux(
    io.wen && is_mcycle,
    io.wdata,
    Mux(io.ok_to_step, csr_mcycle + 1.U(32.W), csr_mcycle)
  )

  val csr_mstatus = RegEnable(io.wdata, "h1800".U(32.W), io.wen && is_mstatus)
  val csr_mcause =
    RegEnable(
      Mux(io.interruption, io.new_cause, io.wdata),
      (io.wen && is_mcause) || io.interruption
    )
  val csr_mtvec = RegEnable(io.wdata, io.wen && is_mtvec)
  val csr_mepc = RegEnable(
    Mux(io.interruption, io.cur_pc, io.wdata),
    (io.wen && is_mepc) || io.interruption
  )

  io.rdata := Mux1H(
    Seq(
      is_mvendorid -> csr_mvendorid,
      is_marchid -> csr_marchid,
      is_mcycle -> csr_mcycle,
      is_mstatus -> csr_mstatus,
      is_mcause -> csr_mcause,
      is_mtvec -> csr_mtvec,
      is_mepc -> csr_mepc
    )
  )

  io.mepc := csr_mepc
  io.mtvec := csr_mtvec

}
