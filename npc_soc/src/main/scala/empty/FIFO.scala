package empty
import chisel3._
import chisel3.util._
class FIFO(BITWIDTH: Int, DEPTH_2POW: Int) extends Module {
  val DEPTH = (1 << DEPTH_2POW)
  val io = IO(new Bundle {
    val front = Output(UInt(BITWIDTH.W))
    val wdata = Input(UInt(BITWIDTH.W))
    val push = Input(Bool())
    val pop = Input(Bool())
    val full = Output(Bool())
    val empty = Output(Bool())
    val ptr_w = Output(UInt((DEPTH_2POW + 1).W))
    val ptr_r = Output(UInt((DEPTH_2POW + 1).W))
  })

  val ptr_w_r = RegInit(UInt((DEPTH_2POW + 1).W), 0.U)
  val ptr_r_r = RegInit(UInt((DEPTH_2POW + 1).W), 0.U)
  io.ptr_w := ptr_w_r
  io.ptr_r := ptr_r_r

  io.empty := (ptr_w_r === ptr_r_r)
  io.full := (ptr_w_r(DEPTH_2POW - 1, 0) === ptr_r_r(
    DEPTH_2POW - 1,
    0
  )) && (ptr_w_r(
    DEPTH_2POW
  ) =/= ptr_r_r(DEPTH_2POW))

  val content = Mem(DEPTH, UInt(BITWIDTH.W))
  io.front := content.read(ptr_r_r(DEPTH_2POW - 1, 0))

  when(io.push) {
    content.write(ptr_w_r(DEPTH_2POW - 1, 0), io.wdata)
    ptr_w_r := ptr_w_r + 1.U
  }

  when(io.pop) {
    ptr_r_r := ptr_r_r + 1.U
  }
}
