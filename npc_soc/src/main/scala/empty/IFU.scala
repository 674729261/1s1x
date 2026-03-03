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
  val out = IO(DecoupledIO(new MessageIFU2IDU))
  val exu_dnpc_fire = in.exu_dnpc_valid && in.exu_dnpc_ready

  fetch_port <> icache.fetch_port
  icache.io.valid := (out.fire || !has_inst)
  icache.io.addr := Mux(
    has_inst && !exu_dnpc_fire,
    fetch_pc + 4.U(32.W),
    fetch_pc
  )
  icache.io.clear := in.clear_icache_valid
  in.clear_icache_ok := icache.io.clear_ok

  val cache_fire = icache.io.valid && icache.io.ready
  has_inst := MuxCase(
    has_inst,
    Seq(
      (exu_dnpc_fire || (out.fire && !cache_fire)) -> false.B,
      (cache_fire && !out.fire) -> true.B
    )
  )

  val inst_reg =
    RegEnable(icache.io.rdata, cache_fire)
  fetch_pc := MuxCase(
    fetch_pc,
    Seq(
      exu_dnpc_fire -> in.exu_dnpc,
      (out.fire && !exu_dnpc_fire) -> (fetch_pc + 4.U(32.W))
    )
  )

  in.exu_dnpc_ready := has_inst

  out.valid := has_inst && !exu_dnpc_fire

  out.bits.inst := inst_reg
  out.bits.pc := fetch_pc

}
