package empty
import chisel3._
import chisel3.util._
import chisel3.layer._

class npc_top_iverilog(init_pc: Long, performance_counter: Boolean)
    extends Module {
  val io = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val ebreak = Output(Bool())
    val ok_to_step = Output(Bool())

    // val ifu_inst = Input(UInt(32.W))
    // val ifu_addr = Output(UInt(32.W))
    // val ifu_valid = Output(Bool())

    // val lsu_raddr = Output(UInt(32.W))
    // val lsu_waddr = Output(UInt(32.W))
    // val lsu_wdata = Output(UInt(32.W))
    // val lsu_wmask = Output(UInt(4.W))
    // val lsu_valid = Output(Bool())
    // val lsu_wen = Output(Bool())
    // val lsu_rdata = Input(UInt(32.W))

  })

  val cpu = Module(
    new CPU_Core(
      init_pc = init_pc.U(32.W),
      performance_counter = performance_counter
    )
  )
  // val __inst_fetch = Module(new __inst_fetch_bus)
  val fetcher = Module(new __sim_bus())
  block(Verifying) {
    val ebreaker = Module(new Ebreaker)
    val axi_checker = Module(new AXI_Checker)
    val inst_retire = Module(new Inst_Retire)
    inst_retire.clock := clock
    inst_retire.reset := reset
    inst_retire.pc := cpu.io.retire_pc
    inst_retire.retire := cpu.io.ok_to_step
    inst_retire.inst := cpu.io.retire_inst

    axi_checker.clock := clock
    axi_checker.reset := reset
    axi_checker.bvalid := cpu.io.axi_bus.b.valid
    axi_checker.bready := cpu.io.axi_bus.b.ready
    axi_checker.bresp := cpu.io.axi_bus.b.resp
    axi_checker.rvalid := cpu.io.axi_bus.r.valid
    axi_checker.rready := cpu.io.axi_bus.r.ready
    axi_checker.rresp := cpu.io.axi_bus.r.resp

    ebreaker.clock := clock
    ebreaker.reset := reset
    ebreaker.ebreak := cpu.io.ebreak
  }

  val tb_io = IO(new Bundle {
    val raddr = Output(UInt(32.W))
    val waddr = Output(UInt(32.W))
    val wdata = Output(UInt(32.W))
    val wmask = Output(UInt(4.W))
    val wen = Output(Bool())
    val valid = Output(Bool())
    val rdata = Input(UInt(32.W))
  })

  tb_io.raddr := fetcher.io.raddr
  tb_io.valid := fetcher.io.valid
  tb_io.waddr := fetcher.io.waddr
  tb_io.wdata := fetcher.io.wdata
  tb_io.wen := fetcher.io.wen
  tb_io.wmask := fetcher.io.wmask
  fetcher.io.rdata := tb_io.rdata

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

}
