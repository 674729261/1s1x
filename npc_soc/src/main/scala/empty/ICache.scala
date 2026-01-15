package empty

import chisel3._
import chisel3.util._
import chisel3.layer.block

class PerformanceCounter_ICache extends ExtModule {
  val clock = IO(Input(Clock()))
  val reset = IO(Input(Reset()))
  val icache_hit = IO(Input(Bool()))

}

class CacheLine(linesize_2pow: Int, linecount_2pow: Int) extends Bundle {
  val words = (1 << (linesize_2pow - 2))
  val tag_width = 32 - linesize_2pow - linecount_2pow
  val tag = UInt(tag_width.W)
  val data = Vec(words, UInt(32.W))

}

object ShouldCache {
  def apply(addr: UInt): Bool = {
    val high_4bit = addr(31, 28)
    return high_4bit === 0x8.U(4.W) || high_4bit === 0xa.U(
      4.W
    ) || high_4bit === 0xb.U(4.W)
  }
}

class ICache(linesize_2pow: Int, linecount_2pow: Int) extends Module {
  val io = IO(new Bundle {
    val addr = Input(UInt(32.W))
    val valid = Input(Bool())
    val rdata = Output(UInt(32.W))
    val ready = Output(Bool())
  })
  val fetch_port = IO(new AXI)
  set_AXIfull_zero(fetch_port)

  val input_tag = io.addr(31, linecount_2pow + linesize_2pow)
  val input_cache_index =
    io.addr(linecount_2pow + linesize_2pow - 1, linesize_2pow)
  val input_index_inside_cacheline = io.addr(linesize_2pow - 1, 2)

  val bytes = (1 << linesize_2pow)
  val words = (1 << (linesize_2pow - 2))
  val line_count = (1 << linecount_2pow)
  val content =
    Mem(line_count, new CacheLine(linesize_2pow, linecount_2pow))
  val valid_flags =
    RegInit(Vec(line_count, Bool()), VecInit(Seq.fill(line_count)(false.B)))

  val cache_rdata =
    content.read(input_cache_index)
  val ar_fire = fetch_port.ar.valid && fetch_port.ar.ready
  val r_fire = fetch_port.r.valid && fetch_port.r.ready
  val r_fire_last = r_fire && fetch_port.r.last

  val out_ar = RegInit(Bool(), false.B)
  val has_r = RegInit(Bool(), false.B)

  val ifu_fire = io.valid && io.ready

  out_ar := MuxCase(
    out_ar,
    Seq(
      ar_fire -> true.B,
      ifu_fire -> false.B
    )
  )

  has_r := MuxCase(
    has_r,
    Seq(
      r_fire_last -> true.B,
      ifu_fire -> false.B
    )
  )

  val should_cache = ShouldCache(io.addr)

  val in_cache = should_cache && valid_flags(
    input_cache_index
  ) && cache_rdata.tag === input_tag
  fetch_port.ar.valid := !out_ar && !in_cache && io.valid
  fetch_port.r.ready := out_ar && !has_r
  io.ready := io.valid && (in_cache || has_r)

  val axi_rdata_latched_next = Wire(Vec(words, UInt(32.W)))
  val axi_rdata_latched = RegEnable(axi_rdata_latched_next, r_fire)
  axi_rdata_latched_next(words - 1) := fetch_port.r.data
  for (i <- 0 until (words - 1))
    axi_rdata_latched_next(i) := axi_rdata_latched(i + 1)

  io.rdata := Mux(
    should_cache,
    Mux(
      in_cache,
      cache_rdata.data(input_index_inside_cacheline),
      axi_rdata_latched(input_index_inside_cacheline)
    ),
    axi_rdata_latched_next(words - 1)
  )
  io.ready := io.valid && (has_r || in_cache)

  val cache_wdata = Wire(new CacheLine(linesize_2pow, linecount_2pow))
  cache_wdata.data := axi_rdata_latched.asTypeOf(Vec(words, UInt(32.W)))
  cache_wdata.tag := input_tag

  when(!in_cache && ifu_fire && should_cache) {
    content.write(input_cache_index, cache_wdata)
    valid_flags(input_cache_index) := true.B
  }

  fetch_port.ar.addr := Mux(
    should_cache,
    Cat(io.addr(31, linesize_2pow), 0.U(linesize_2pow.W)),
    io.addr
  )

  fetch_port.aw.id := "b0000".U(4.W)
  fetch_port.ar.id := "b0000".U(4.W)
  fetch_port.w.last := true.B
  fetch_port.ar.len := Mux(should_cache, (words - 1).U(8.W), 0.U(8.W))

  block(PerformanceCounterLayer) {
    val performancecounter_icache = Module(new PerformanceCounter_ICache)
    performancecounter_icache.clock := clock
    performancecounter_icache.reset := reset
    performancecounter_icache.icache_hit := ifu_fire && in_cache
  }

  block(AXIAssertLayer) {
    check_signal_stable(
      fetch_port.ar.ready,
      fetch_port.ar.valid,
      Cat(
        fetch_port.ar.addr,
        fetch_port.ar.burst,
        fetch_port.ar.id,
        fetch_port.ar.len,
        fetch_port.ar.size
      ),
      "IFU.ar"
    )
    assert(!fetch_port.aw.valid && !fetch_port.w.valid, "ifu should not write")
    when(r_fire) {
      // assert(fetch_port.r.last, "ifu.axi.rlast is not set")
      assert(fetch_port.r.resp === "b00".U, "ifu.axi.rresp is not b00")
      assert(fetch_port.r.id === "b0000".U, "ifu.axi.rid is not b0000")

    }
  }

}
