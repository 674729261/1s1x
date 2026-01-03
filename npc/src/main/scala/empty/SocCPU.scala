package empty
import chisel3._
import chisel3.util._

class ysyx_25080216 extends Module {
  val io = IO(new Bundle {
    val interrupt = Input(Bool())
    val master = new AXI
    val slave = Flipped(new AXI)
  })
  val cpu = Module(new CPU(init_pc = "h80000000".U(32.W)))
  io.master <> cpu.io.axi_bus
  set_flipped_AXIfull_zero(io.slave)
}
