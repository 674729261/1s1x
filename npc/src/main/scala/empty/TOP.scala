package empty
import chisel3._
import chisel3.util._
import empty.IFU
import empty.IDU
import empty.EXU
import empty.WBU

class CPU(init_pc: UInt) extends Module with RequireAsyncReset {
  val io = IO(new Bundle {
    val instr = Input(UInt(32.W))
    val pc = Output(UInt(32.W))

    val ebreak = Output(Bool())
    val valid = Output(Bool())
    val raddr = Output(UInt(32.W))
    val wen = Output(Bool())
    val waddr = Output(UInt(32.W))
    val wdata = Output(UInt(32.W))
    val wmask = Output(UInt(4.W))

    val rdata = Input(UInt(32.W))

  })

  val ifu = Module(new IFU)
  val idu = Module(new IDU)
  val exu = Module(new EXU)
  val wbu = Module(new WBU)

  val pc = RegNext(next = Cat(wbu.out.dnpc(31, 1), 0.U(1.W)), init = init_pc)
  val gpr = Module(new GPR(CNT = 32, BITWIDTH = 32))
  val csrBank = Module(new CSR)
  val ramWriter = Module(new RamWriteData)
  val ramLoader = Module(new RamLoadData)

  io.pc := pc

  ifu.in.pc := pc
  ifu.in.inst_fetch := io.instr

  idu.in <> ifu.out
  exu.in <> idu.out
  wbu.in <> exu.out

  exu.fetch_port_in.csr_rdata := csrBank.io.rdata
  exu.fetch_port_in.csr_mepc := csrBank.io.mepc
  exu.fetch_port_in.csr_mtvec := csrBank.io.mtvec
  exu.fetch_port_in.gpr_rdata1 := gpr.io.rdata1
  exu.fetch_port_in.gpr_rdata2 := gpr.io.rdata2
  exu.fetch_port_in.mem_rdata := ramLoader.io.out

  gpr.io.raddr1 := exu.fetch_port_out.gpr_raddr1
  gpr.io.raddr2 := exu.fetch_port_out.gpr_raddr2
  gpr.io.waddr := wbu.out.gpr_waddr
  gpr.io.wdata := wbu.out.gpr_wdata
  gpr.io.wen := wbu.out.gpr_wen

  csrBank.io.csr := wbu.out.csr_waddr
  csrBank.io.cur_pc := wbu.out.csr_cur_pc
  csrBank.io.interruption := wbu.out.csr_interruption
  csrBank.io.wdata := wbu.out.csr_wdata
  csrBank.io.new_cause := wbu.out.csr_mcause
  csrBank.io.wen := wbu.out.csr_wen

  ramWriter.io.word := wbu.out.mem_wdata
  ramWriter.io.is_word := wbu.out.is_word
  ramWriter.io.is_half := wbu.out.is_half
  ramWriter.io.is_byte := wbu.out.is_byte
  ramWriter.io.lower2bit := wbu.out.mem_lower2bit
  io.wdata := ramWriter.io.out
  io.wmask := ramWriter.io.mask
  io.waddr := wbu.out.mem_waddr
  io.raddr := exu.fetch_port_out.mem_raddr
  io.valid := exu.fetch_port_out.mem_rvalid | wbu.out.mem_wen
  io.wen := wbu.out.mem_wen

  ramLoader.io.is_byte := exu.fetch_port_out.is_byte
  ramLoader.io.is_half := exu.fetch_port_out.is_half
  ramLoader.io.is_word := exu.fetch_port_out.is_word
  ramLoader.io.is_unsigned := exu.fetch_port_out.is_unsigned
  // ramLoader.io.word := memory_proxy.io.rdata
  ramLoader.io.word := io.rdata

  ramLoader.io.lower2bit := exu.fetch_port_out.mem_rlower2bit
  io.ebreak := wbu.out.ebreak
}
