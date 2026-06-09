package empty

import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageIFU2IDU extends Bundle {
  val pc = UInt(32.W)
  val inst = UInt(32.W)
  val in_cache = Bool()
}

class IFU(init_pc: UInt) extends PrefixedModule {
  val in = IO(new Bundle {
    val exu_dnpc = Input(UInt(32.W))
    val flush_valid = Input(Bool())
    val fencei = Input(Bool())
  })

  val fetch_port = IO(new AXI)
  val icache = Module(new ICache(3, 1))

  val has_inst_r = RegInit(Bool(), false.B)
  val fetch_pc_r = RegInit(UInt(32.W), init_pc)
  val redirect_pending_r = RegInit(Bool(), false.B)
  val redirect_pc_r = RegInit(UInt(32.W), init_pc)
  val timestamp_r = RegInit(UInt(3.W), 0.U(3.W))

  val out = IO(DecoupledIO(new MessageIFU2IDU))

  val timestamp_next = (timestamp_r + 1.U)(2, 0)
  val timestamp = Mux(in.flush_valid, timestamp_next, timestamp_r)
  val redirect_active = in.flush_valid || redirect_pending_r
  val fetch_addr = Mux(in.flush_valid, in.exu_dnpc, Mux(redirect_pending_r, redirect_pc_r, fetch_pc_r))
  val pc_next = fetch_addr + 4.U(32.W)
  val has_inst = has_inst_r && !redirect_active

  fetch_port <> icache.fetch_port
  icache.io.addr := fetch_addr
  icache.io.timestamp_req := timestamp
  icache.io.avalid := !in.flush_valid
  icache.io.rready := out.fire || !has_inst
  icache.io.clear := in.fencei && in.flush_valid

  val cache_afire = icache.io.avalid && icache.io.aready
  val cache_rfire = icache.io.rvalid && icache.io.rready
  val should_discard = icache.io.timestamp_res =/= timestamp
  val cache_rfire_and_up_to_date = cache_rfire && !should_discard
  val redirect_accept = redirect_pending_r && cache_afire

  has_inst_r := MuxCase(
    has_inst_r,
    Seq(
      (cache_rfire_and_up_to_date && !out.fire && !redirect_active) -> true.B,
      (redirect_active || (out.fire && !cache_rfire_and_up_to_date)) -> false.B
    )
  )

  val inst_reg = RegEnable(icache.io.rdata, cache_rfire)
  val inst_rpc = RegEnable(icache.io.rpc, cache_rfire)
  val inst_incache = RegEnable(icache.io.in_cache, cache_rfire)

  fetch_pc_r := MuxCase(
    fetch_pc_r,
    Seq(
      in.flush_valid -> in.exu_dnpc,
      cache_afire -> pc_next
    )
  )

  redirect_pc_r := Mux(in.flush_valid, in.exu_dnpc, redirect_pc_r)
  redirect_pending_r := MuxCase(
    redirect_pending_r,
    Seq(
      in.flush_valid -> true.B,
      redirect_accept -> false.B
    )
  )
  timestamp_r := Mux(in.flush_valid, timestamp_next, timestamp_r)

  out.valid := has_inst
  out.bits.inst := inst_reg
  out.bits.pc := inst_rpc
  out.bits.in_cache := inst_incache
}
