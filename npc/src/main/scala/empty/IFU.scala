package empty.empty
import chisel3._
import chisel3.util._

class IFU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val inst_fetch = Input(UInt(32.W))
  })

  val out = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val inst = Output(UInt(32.W))
  })
  out.inst := in.inst_fetch
  out.pc := in.pc

}
