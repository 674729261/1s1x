package empty

import chisel3._
import chisel3.util._

class CSR extends PrefixedModule {
  val io = IO(new Bundle {
    val csr_w = Input(UInt(12.W))
    val csr_r = Input(UInt(12.W))

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

  val is_mvendorid_r = io.csr_r === 0xf11.U(12.W)
  val is_marchid_r = io.csr_r === 0xf12.U(12.W)
  val is_mcycle_r = io.csr_r === 0xb00.U(12.W)
  val is_mcycleh_r = io.csr_r === 0xb80.U(12.W)
  val is_mstatus_r = io.csr_r === 0x300.U(12.W)
  val is_mcause_r = io.csr_r === 0x342.U(12.W)
  val is_mtvec_r = io.csr_r === 0x305.U(12.W)
  val is_mepc_r = io.csr_r === 0x341.U(12.W)

  val is_mvendorid_w = io.csr_w === 0xf11.U(12.W)
  val is_marchid_w = io.csr_w === 0xf12.U(12.W)
  val is_mcycle_w = io.csr_w === 0xb00.U(12.W)
  val is_mcycleh_w = io.csr_w === 0xb80.U(12.W)
  val is_mstatus_w = io.csr_w === 0x300.U(12.W)
  val is_mcause_w = io.csr_w === 0x342.U(12.W)
  val is_mtvec_w = io.csr_w === 0x305.U(12.W)
  val is_mepc_w = io.csr_w === 0x341.U(12.W)

  val csr_mvendorid = 0x79737978.U(32.W)
  val csr_marchid = 0x17eb198.U(32.W)

  val mcycle_nxt = Wire(Vec(2, UInt(32.W)))
  val mcycle_inc = Wire(Vec(2, UInt(32.W)))
  val csr_mcycle = RegNext(mcycle_nxt, VecInit(Seq(0.U(32.W), 0.U(32.W))))
  mcycle_inc := (csr_mcycle.asUInt + 1.U(64.W)).asTypeOf(Vec(2, UInt(32.W)))
  mcycle_nxt(0) := Mux(
    io.wen && is_mcycle_w,
    io.wdata,
    mcycle_inc(0)
  )
  mcycle_nxt(1) := Mux(
    io.wen && is_mcycleh_w,
    io.wdata,
    mcycle_inc(1)
  )

  val csr_mstatus = RegEnable(io.wdata, "h1800".U(32.W), io.wen && is_mstatus_w)
  val csr_mcause =
    RegEnable(
      Mux(io.interruption, io.new_cause, io.wdata),
      (io.wen && is_mcause_w) || io.interruption
    )
  val csr_mtvec = RegEnable(io.wdata, io.wen && is_mtvec_w)
  val csr_mepc = RegEnable(
    Mux(io.interruption, io.cur_pc, io.wdata),
    (io.wen && is_mepc_w) || io.interruption
  )

  io.rdata := Mux1H(
    Seq(
      is_mvendorid_r -> csr_mvendorid,
      is_marchid_r -> csr_marchid,
      is_mcycle_r -> csr_mcycle(0),
      is_mcycleh_r -> csr_mcycle(1),
      is_mstatus_r -> csr_mstatus,
      is_mcause_r -> csr_mcause,
      is_mtvec_r -> csr_mtvec,
      is_mepc_r -> csr_mepc
    )
  )

  io.mepc := csr_mepc
  io.mtvec := csr_mtvec

}
