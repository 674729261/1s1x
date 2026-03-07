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
    val flush_valid = Input(Bool())
  })

  val fetch_port = IO(new AXI)

  val icache = Module(new ICache(5, 1))
  val has_inst_r = RegInit(Bool(), false.B)

  val fetch_pc = RegInit(UInt(32.W), init_pc)
  val out = IO(DecoupledIO(new MessageIFU2IDU))
  val cache_fire = icache.io.valid && icache.io.ready

  val pc_next_predicted = fetch_pc + 4.U(32.W)
  val pending_flush_r = RegInit(Bool(), false.B)
  pending_flush_r := MuxCase(
    pending_flush_r,
    Seq(
      in.flush_valid -> true.B,
      cache_fire -> false.B
    )
  )
  val pending_flush = pending_flush_r || in.flush_valid

  val has_inst = has_inst_r && !pending_flush

  fetch_port <> icache.fetch_port
  icache.io.valid := (out.fire || !has_inst)
  icache.io.addr := Mux(
    has_inst,
    pc_next_predicted,
    fetch_pc
  )
  icache.io.clear := false.B

  has_inst_r := MuxCase(
    has_inst_r,
    Seq(
      (cache_fire && !out.fire) -> true.B,
      (pending_flush || (out.fire && !cache_fire)) -> false.B
    )
  )

  val inst_reg =
    RegEnable(icache.io.rdata, cache_fire)
  fetch_pc := MuxCase(
    fetch_pc,
    Seq(
      (out.fire) -> pc_next_predicted
    )
  )

  out.valid := has_inst

  out.bits.inst := inst_reg
  out.bits.pc := fetch_pc

}
