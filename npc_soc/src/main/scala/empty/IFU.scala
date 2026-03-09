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

  val icache = Module(new ICache(5, 2))
  val has_inst_r = RegInit(Bool(), false.B)

  val fetch_pc_r = RegInit(UInt(32.W), init_pc)
  val fetch_pc = Mux(in.flush_valid, in.exu_dnpc, fetch_pc_r)
  val out = IO(DecoupledIO(new MessageIFU2IDU))
  val cache_afire = icache.io.avalid && icache.io.aready
  val cache_rfire = icache.io.rvalid && icache.io.rready

  val timestamp_r = RegInit(UInt(3.W), 0.U(3.W))
  val timestamp = Mux(in.flush_valid, timestamp_r + 1.U(3.W), timestamp_r)

  val pc_next_predicted = fetch_pc_r + 4.U(32.W)

  val has_inst = has_inst_r && !in.flush_valid

  fetch_port <> icache.fetch_port
  icache.io.addr := fetch_pc
  icache.io.timestamp_req := timestamp
  icache.io.avalid := true.B
  icache.io.rready := out.fire || !has_inst

  val cache_rfire_and_up_to_date =
    cache_rfire && (icache.io.timestamp_res === timestamp)

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
  fetch_pc_r := MuxCase(
    fetch_pc_r,
    Seq(
      (in.flush_valid) -> Mux(
        cache_afire,
        in.exu_dnpc + 4.U(32.W),
        in.exu_dnpc
      ),
      (cache_afire) -> pc_next_predicted
    )
  )

  timestamp_r := MuxCase(
    timestamp_r,
    Seq(in.flush_valid -> (timestamp_r + 1.U(3.W)))
  )

  out.valid := has_inst

  out.bits.inst := inst_reg
  out.bits.pc := inst_rpc

}
