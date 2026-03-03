package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
}

class IFU(init_pc: UInt) extends Module {
  val in = IO(new Bundle {
    val exu_dnpc = Input(UInt(32.W))
    val exu_dnpc_valid = Input(Bool())
    val exu_dnpc_ready = Output(Bool())
    val clear_icache_valid = Input(Bool())
    val clear_icache_ok = Output(Bool())
  })

  val fetch_port = IO(new AXI)

  val icache = Module(new ICache(4, 2))
  val has_inst = RegInit(Bool(), false.B)

  val fetch_pc = RegInit(UInt(32.W), init_pc)

  fetch_port <> icache.fetch_port
  icache.io.valid := !has_inst
  icache.io.addr := fetch_pc
  icache.io.clear := in.clear_icache_valid
  in.clear_icache_ok := icache.io.clear_ok

  val out = IO(DecoupledIO(new MessageIFU2IDU))

  val cache_fire = icache.io.valid && icache.io.ready
  // when cache hits we can forward rdata this cycle (bypass);
  // otherwise we store rdata/pc into regs for later delivery
  val inst_reg = RegEnable(icache.io.rdata, cache_fire)
  val inst_pc_reg = RegEnable(fetch_pc, cache_fire)

  // set has_inst only when cache produced data but it was NOT consumed this cycle
  has_inst := MuxCase(
    has_inst,
    Seq(
      (cache_fire && !out.fire) -> true.B,
      out.fire -> false.B
    )
  )
  val exu_dnpc_fire = in.exu_dnpc_valid && in.exu_dnpc_ready
  fetch_pc := MuxCase(
    fetch_pc,
    Seq(
      exu_dnpc_fire -> in.exu_dnpc,
      (cache_fire && !exu_dnpc_fire) -> (fetch_pc + 4.U(32.W))
    )
  )

  in.exu_dnpc_ready := has_inst

  // valid when we have buffered inst or when cache is returning this cycle
  out.valid := has_inst || cache_fire

  // bypass path: if cache_fire this cycle take icache.rdata/fetch_pc directly,
  // otherwise use the buffered registers
  out.bits.inst := Mux(cache_fire, icache.io.rdata, inst_reg)
  out.bits.pc := Mux(cache_fire, fetch_pc, inst_pc_reg)

}
