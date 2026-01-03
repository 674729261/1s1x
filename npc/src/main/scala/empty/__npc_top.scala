package empty
import chisel3._
import chisel3.util._
import firrtl.annotations

class Printer extends ExtModule {
  val clock = IO(Input(Clock()))
  val valid = IO(Input(Bool()))
  val data = IO(Input(UInt(8.W)))
}

class UART extends Module {
  val fetch_port = IO(Flipped(new AXI_Lite))
  val sIDLE :: sRESP :: Nil = Enum(2)
  val state_r = RegInit(sIDLE)

  val w_fire = fetch_port.w.wvalid && fetch_port.w.wready
  val b_fire = fetch_port.b.bvalid && fetch_port.b.bready

  fetch_port.ar.arready := false.B
  fetch_port.r.rvalid := false.B
  fetch_port.r.rdata := 0.U(32.W)
  fetch_port.r.rresp := "b10".U(2.W)
  fetch_port.aw.awready := state_r === sIDLE
  fetch_port.w.wready := state_r === sIDLE
  fetch_port.b.bvalid := state_r === sRESP
  fetch_port.b.bresp := "b00".U(2.W)

  state_r := MuxLookup(state_r, sIDLE)(
    Seq(sIDLE -> Mux(w_fire, sRESP, sIDLE), sRESP -> Mux(b_fire, sIDLE, sRESP))
  )
  val char_lower8 = fetch_port.w.wdata(7, 0)
  val printer = Module(new Printer)

  printer.clock := clock
  printer.valid := w_fire
  printer.data := char_lower8
}

class CLINT extends Module {
  val fetch_port = IO(Flipped(new AXI_Lite))
  val sIDLE :: sRESP :: Nil = Enum(2)
  val state_r = RegInit(sIDLE)

  val w_fire = fetch_port.ar.arvalid && fetch_port.ar.arready
  val b_fire = fetch_port.r.rvalid && fetch_port.r.rready

  fetch_port.ar.arready := false.B
  fetch_port.r.rvalid := false.B
  fetch_port.r.rdata := 0.U(32.W)
  fetch_port.r.rresp := "b10".U(2.W)
  fetch_port.aw.awready := state_r === sIDLE
  fetch_port.w.wready := state_r === sIDLE
  fetch_port.b.bvalid := state_r === sRESP
  fetch_port.b.bresp := "b00".U(2.W)

  state_r := MuxLookup(state_r, sIDLE)(
    Seq(sIDLE -> Mux(w_fire, sRESP, sIDLE), sRESP -> Mux(b_fire, sIDLE, sRESP))
  )
}

class npc_top extends Module {
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

  val cpu = Module(new CPU(init_pc = "h80000000".U(32.W)))
  // val __inst_fetch = Module(new __inst_fetch_bus)
  val __lsu_fetch = Module(new __lsu_fetch_bus)
  // val serial = Module(new UART)
  // val clint = Module(new CLINT)
  // val crossbar = Module(new Xbar)
  // cpu.io.inst_bus_axi <> __inst_fetch.fetch_port
  cpu.io.axi_bus <> __lsu_fetch.fetch_port
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

  io.lsu_raddr := __lsu_fetch.io.raddr
  io.lsu_valid := __lsu_fetch.io.valid
  io.lsu_waddr := __lsu_fetch.io.waddr
  io.lsu_wdata := __lsu_fetch.io.wdata
  io.lsu_wen := __lsu_fetch.io.wen
  io.lsu_wmask := __lsu_fetch.io.wmask
  __lsu_fetch.io.rdata := io.lsu_rdata

}
