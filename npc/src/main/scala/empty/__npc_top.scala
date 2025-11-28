package empty.empty
import chisel3._
import chisel3.util._
import empty.InstBus
import _root_.empty.CPU
import _root_.empty.InstBus
import _root_.empty.MemAccessBus

class npc_top() extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val ebreak = Output(Bool())
    val ok_to_step = Output(Bool())

    val ifu_inst = Input(UInt(32.W))
    val ifu_addr = Output(UInt(32.W))
    val ifu_valid = Output(Bool())

    val lsu_raddr = Output(UInt(32.W))
    val lsu_waddr = Output(UInt(32.W))
    val lsu_wdata = Output(UInt(32.W))
    val lsu_wmask = Output(UInt(4.W))
    val lsu_valid = Output(Bool())
    val lsu_wen = Output(Bool())
    val lsu_rdata = Input(UInt(32.W))

  })

  val cpu = Module(new CPU(init_pc = "h80000000".U(32.W)))
  val __inst_fetch = Module(new __inst_fetch_bus)
  val __lsu_fetch = Module(new __lsu_fetch_bus)
  cpu.io.inst_bus_axi <> __inst_fetch.fetch_port
  cpu.io.mem <> __lsu_fetch.fetch_port

  io.pc := cpu.io.pc
  io.ebreak := cpu.io.ebreak
  io.ok_to_step := cpu.io.ok_to_step

  io.ifu_addr := __inst_fetch.io.addr
  io.ifu_valid := __inst_fetch.io.valid
  __inst_fetch.io.instr := io.ifu_inst

  io.lsu_raddr := __lsu_fetch.io.raddr
  io.lsu_valid := __lsu_fetch.io.valid
  io.lsu_waddr := __lsu_fetch.io.waddr
  io.lsu_wdata := __lsu_fetch.io.wdata
  io.lsu_wen := __lsu_fetch.io.wen
  io.lsu_wmask := __lsu_fetch.io.wmask
  __lsu_fetch.io.rdata := io.lsu_rdata

}
