package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
}

class IFU() extends Module {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
  })

  val fetch_port = IO(new AXI)

  val icache = Module(new ICache(4, 3))
  val has_inst = RegInit(Bool(), false.B)
  fetch_port <> icache.fetch_port
  icache.io.valid := !has_inst
  icache.io.addr := in.pc

  val out = IO(DecoupledIO(new MessageIFU2IDU))
  val cpu_out_fire = out.valid && out.ready
  val cache_fire = icache.io.valid && icache.io.ready
  has_inst := MuxCase(
    has_inst,
    Seq(
      cache_fire -> true.B,
      cpu_out_fire -> false.B
    )
  )

  val inst_reg =
    RegEnable(icache.io.rdata, cache_fire)

  out.valid := has_inst

  out.bits.inst := inst_reg
  out.bits.pc := in.pc

}
