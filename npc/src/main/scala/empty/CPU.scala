package empty
import chisel3._
import chisel3.util._
import empty.IFU
import empty.IDU
import empty.EXU
import empty.WBU
import _root_.empty.empty.LSU

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

class InstBus extends Bundle {
  val ifu_addr = Output(UInt(32.W))
  val instr = Input(UInt(32.W))

  val reqValid = Output(Bool())
  val reqReady = Input(Bool())
  val respValid = Input(Bool())
  val respReady = Output(Bool())
}
class CPU(init_pc: UInt) extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val inst_bus = new InstBus
    val ebreak = Output(Bool())
    val mem = new MemAccessBus
    val ok_to_step = Output(Bool())
  })

  val ifu = Module(new IFU)
  val idu = Module(new IDU)
  val exu = Module(new EXU)
  val lsu = Module(new LSU)
  val wbu = Module(new WBU)

  val pc = RegNext(next = Cat(wbu.out.dnpc(31, 1), 0.U(1.W)), init = init_pc)
  val gpr = Module(new GPR(CNT = 32, BITWIDTH = 32))
  val csrBank = Module(new CSR)

  io.pc := pc

  ifu.in.pc := pc
  ifu.fetch_port <> io.inst_bus

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

  lsu.fetch_port <> io.mem

  gpr.io.raddr1 := idu.fetch_port_out.gpr_raddr1
  gpr.io.raddr2 := idu.fetch_port_out.gpr_raddr2
  gpr.io.waddr := wbu.out.gpr_waddr
  gpr.io.wdata := wbu.out.gpr_wdata
  gpr.io.wen := wbu.out.gpr_wen

  csrBank.io.csr := wbu.out.csr_waddr
  csrBank.io.cur_pc := wbu.out.csr_cur_pc
  csrBank.io.interruption := wbu.out.csr_interruption
  csrBank.io.wdata := wbu.out.csr_wdata
  csrBank.io.new_cause := wbu.out.csr_mcause
  csrBank.io.wen := wbu.out.csr_wen

  // val ebreak_reg = RegInit(false.B)
  // ebreak_reg := ebreak_reg | wbu.out.ebreak
  io.ebreak := wbu.out.ebreak
}
