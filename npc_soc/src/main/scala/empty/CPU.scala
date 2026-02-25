package empty
import chisel3._
import chisel3.util._
import chisel3.layer._

object PerformanceCounterLayer extends Layer(LayerConfig.Inline)

object StageConnect {
  def apply[T <: Data](left: DecoupledIO[T], right: DecoupledIO[T]) = {
    val arch = "multi"
    if (arch == "single") { right.bits := left.bits }
    else if (arch == "multi") { right <> left }
    else if (arch == "pipeline") { right <> RegEnable(left, left.fire) }
    else if (arch == "ooo") { right <> Queue(left, 16) }
  }
}

class MemAccessBus extends Bundle {
  val raddr = Output(UInt(32.W))
  val wen = Output(Bool())
  val waddr = Output(UInt(32.W))
  val wdata = Output(UInt(32.W))
  val wmask = Output(UInt(4.W))
  val rdata = Input(UInt(32.W))

  val reqValid = Output(Bool())
  val reqReady = Input(Bool())
  val respValid = Input(Bool())
  val respReady = Output(Bool())
}

class PerformanceCounter extends ExtModule {
  val clock = IO(Input(Clock()))
  val reset = IO(Input(Reset()))
  val pc = IO(Input(UInt(32.W)))
  val ifu_arready = IO(Input(Bool()))
  val ifu_arvalid = IO(Input(Bool()))
  val ifu_rready = IO(Input(Bool()))
  val ifu_rvalid = IO(Input(Bool()))
  val lsu_arready = IO(Input(Bool()))
  val lsu_arvalid = IO(Input(Bool()))
  val lsu_rready = IO(Input(Bool()))
  val lsu_rvalid = IO(Input(Bool()))
  val exu_ready = IO(Input(Bool()))
  val exu_valid = IO(Input(Bool()))
  val idu_ready = IO(Input(Bool()))
  val idu_valid = IO(Input(Bool()))
  val wbu_valid = IO(Input(Bool()))
  val inst_type = IO(Input(new InstType))
}

class CPU_Core(init_pc: UInt, performance_counter: Boolean) extends Module {
  val io = IO(new Bundle {
    val pc = Output(UInt(32.W))
    // val inst_bus_axi = new AXI_Lite
    val ebreak = Output(Bool())
    // val mem = new AXI_Lite
    val axi_bus = new AXI
    val ok_to_step = Output(Bool())
  })

  val ifu = Module(new IFU)
  val idu = Module(new IDU)
  val exu = Module(new EXU)
  val lsu = Module(new LSU)
  val wbu = Module(new WBU)

  val pc = RegEnable(
    Cat(wbu.out.dnpc(31, 1), 0.U(1.W)),
    init_pc,
    wbu.out.ok_to_step
  )
  val gpr = Module(new GPR(CNT = 32, BITWIDTH = 32))
  val csrBank = Module(new CSR)

  val arbiter = Module(new Arbiter_2Master)

  io.axi_bus <> arbiter.OUT_AXI

  io.pc := pc

  ifu.in.pc := pc
  ifu.fetch_port <> arbiter.IFU_AXI

  StageConnect(idu.in, ifu.out)
  StageConnect(exu.in, idu.out)
  StageConnect(lsu.in, exu.out)
  StageConnect(wbu.in, lsu.out)

  // val ok_to_step_reg = RegInit(false.B)
  // ok_to_step_reg := wbu.out.ok_to_step

  io.ok_to_step := wbu.out.ok_to_step
  csrBank.io.ok_to_step := wbu.out.ok_to_step

  idu.fetch_port_in.csr_rdata := csrBank.io.rdata
  idu.fetch_port_in.csr_mepc := csrBank.io.mepc
  idu.fetch_port_in.csr_mtvec := csrBank.io.mtvec
  idu.fetch_port_in.gpr_rdata1 := gpr.io.rdata1
  idu.fetch_port_in.gpr_rdata2 := gpr.io.rdata2

  lsu.fetch_port <> arbiter.LSU_AXI

  gpr.io.raddr1 := idu.fetch_port_out.gpr_raddr1
  gpr.io.raddr2 := idu.fetch_port_out.gpr_raddr2
  gpr.io.waddr := wbu.out.gpr_waddr
  gpr.io.wdata := wbu.out.gpr_wdata
  gpr.io.wen := wbu.out.gpr_wen

  csrBank.io.csr_w := wbu.out.csr_waddr
  csrBank.io.csr_r := idu.fetch_port_out.csr_raddr
  csrBank.io.cur_pc := wbu.out.csr_cur_pc
  csrBank.io.interruption := wbu.out.csr_interruption
  csrBank.io.wdata := wbu.out.csr_wdata
  csrBank.io.new_cause := wbu.out.csr_mcause
  csrBank.io.wen := wbu.out.csr_wen

  // val ebreak_reg = RegInit(false.B)
  // ebreak_reg := ebreak_reg | wbu.out.ebreak
  io.ebreak := wbu.out.ebreak

  block(PerformanceCounterLayer) {
    val m_performance_counter = Module(new PerformanceCounter)
    m_performance_counter.clock := clock
    m_performance_counter.reset := reset
    m_performance_counter.pc := pc
    m_performance_counter.exu_ready := exu.out.ready
    m_performance_counter.exu_valid := exu.out.valid
    m_performance_counter.idu_ready := idu.out.ready
    m_performance_counter.idu_valid := idu.out.valid
    m_performance_counter.wbu_valid := wbu.out.ok_to_step
    m_performance_counter.inst_type := wbu.out.inst_type

    m_performance_counter.ifu_arready := ifu.fetch_port.ar.ready
    m_performance_counter.ifu_arvalid := ifu.fetch_port.ar.valid
    m_performance_counter.ifu_rready := ifu.fetch_port.r.ready
    m_performance_counter.ifu_rvalid := ifu.fetch_port.r.valid
    m_performance_counter.lsu_arready := lsu.fetch_port.ar.ready
    m_performance_counter.lsu_arvalid := lsu.fetch_port.ar.valid
    m_performance_counter.lsu_rready := lsu.fetch_port.r.ready
    m_performance_counter.lsu_rvalid := lsu.fetch_port.r.valid

  }
}
