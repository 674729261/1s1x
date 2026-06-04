package empty
import chisel3._
import chisel3.util._
import chisel3.layer.block

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
  val in_cache = (Bool())
  val predicted_jump = (Bool())
}

class IFU(init_pc: UInt) extends PrefixedModule {
  val in = IO(new Bundle {
    val exu_dnpc = Input(UInt(32.W))
    val flush_valid = Input(Bool())
    val fencei = Input(Bool())

    val btb_pc = Output(UInt(32.W))
    val btb_nxt_pc = Input(UInt(32.W))
    val btb_jump = Input(Bool())
  })

  val fetch_port = IO(new AXI)

  val icache = Module(new ICache(4, 3))
  val has_inst_r = RegInit(Bool(), false.B)

  val fetch_pc_r = RegInit(UInt(32.W), init_pc)
  val out = IO(DecoupledIO(new MessageIFU2IDU))
  val cache_afire = icache.io.avalid && icache.io.aready
  val cache_rfire = icache.io.rvalid && icache.io.rready

  val timestamp_r = RegInit(UInt(2.W), 0.U(2.W))
  val timestamp = Mux(in.flush_valid, timestamp_r + 1.U(2.W), timestamp_r)

  in.btb_pc := fetch_pc_r
  val pc_next_predicted = in.btb_nxt_pc
  val pc_predicted_jump = in.btb_jump

  val has_inst = has_inst_r && !in.flush_valid
  val should_discard = (icache.io.timestamp_res =/= timestamp);
  fetch_port <> icache.fetch_port
  icache.io.addr := fetch_pc_r
  icache.io.predicted_jump := pc_predicted_jump
  icache.io.timestamp_req := timestamp
  icache.io.avalid := !in.flush_valid
  icache.io.rready := out.fire || !has_inst
  icache.io.clear := in.fencei && in.flush_valid
  val cache_rfire_and_up_to_date =
    cache_rfire && !should_discard

  has_inst_r := MuxCase(
    has_inst_r,
    Seq(
      (cache_rfire_and_up_to_date && !out.fire) -> true.B,
      (in.flush_valid || (out.fire && !cache_rfire_and_up_to_date)) -> false.B
    )
  )

  val inst_reg =
    RegEnable(icache.io.rdata, cache_rfire)
  val inst_rpc =
    RegEnable(icache.io.rpc, cache_rfire)
  val inst_predicted_jump =
    RegEnable(icache.io.rjump, cache_rfire)
  val inst_incache =
    RegEnable(icache.io.in_cache, cache_rfire)

  fetch_pc_r := MuxCase(
    fetch_pc_r,
    Seq(
      (in.flush_valid) -> in.exu_dnpc,
      (cache_afire) -> pc_next_predicted
    )
  )

  timestamp_r := MuxCase(
    timestamp_r,
    Seq(in.flush_valid -> (timestamp_r + 1.U(2.W)))
  )

  out.valid := has_inst

  out.bits.inst := inst_reg
  out.bits.pc := inst_rpc
  out.bits.in_cache := inst_incache
  out.bits.predicted_jump := inst_predicted_jump

}
