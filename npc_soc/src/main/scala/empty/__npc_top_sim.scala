package empty
import chisel3._
import chisel3.util._

class npc_top(init_pc: Long, performance_counter: Boolean) extends Module {
  val io = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val ebreak = Output(Bool())
    val ok_to_step = Output(Bool())

    // val ifu_inst = Input(UInt(32.W))
    // val ifu_addr = Output(UInt(32.W))
    // val ifu_valid = Output(Bool())

    val lsu_raddr = Output(UInt(32.W))
    val lsu_waddr = Output(UInt(32.W))
    val lsu_wdata = Output(UInt(32.W))
    val lsu_wmask = Output(UInt(4.W))
    val lsu_valid = Output(Bool())
    val lsu_wen = Output(Bool())
    val lsu_rdata = Input(UInt(32.W))

  })

  val cpu = Module(
    new CPU_Core(
      init_pc = init_pc.U(32.W),
      performance_counter = performance_counter
    )
  )
  // val __inst_fetch = Module(new __inst_fetch_bus)
  val fetcher = Module(new __sim_bus())

  // val serial = Module(new UART)
  // val clint = Module(new CLINT)
  // val crossbar = Module(new Xbar)
  // cpu.io.inst_bus_axi <> __inst_fetch.fetch_port
  cpu.io.axi_bus <> fetcher.fetch_port
  // cpu.io.axi_bus <> crossbar.IN_AXI
  // crossbar.MEM_AXI <> __lsu_fetch.fetch_port
  // crossbar.UART_AXI <> serial.fetch_port
  // crossbar.CLINT_AXI <> clint.fetch_port

  io.pc := cpu.io.pc
  io.ebreak := cpu.io.ebreak
  io.ok_to_step := cpu.io.ok_to_step

  // io.ifu_addr := __inst_fetch.io.addr
  // io.ifu_valid := __inst_fetch.io.valid
  // __inst_fetch.io.instr := io.ifu_inst

  io.lsu_raddr := fetcher.io.raddr
  io.lsu_valid := fetcher.io.valid
  io.lsu_waddr := fetcher.io.waddr
  io.lsu_wdata := fetcher.io.wdata
  io.lsu_wen := fetcher.io.wen
  io.lsu_wmask := fetcher.io.wmask
  fetcher.io.rdata := io.lsu_rdata

}
