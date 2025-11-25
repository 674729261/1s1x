package empty.empty
import chisel3._
import chisel3.util._

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
}

class IFU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val inst_fetch = Input(UInt(32.W))
  })

  val fetch_port_out = IO(new Bundle {
    val ifu_addr = Output(UInt(32.W))
    val ifu_valid = Output(Bool())
  })

  val out = IO(DecoupledIO(new MessageIFU2IDU))
  out.bits.inst := in.inst_fetch
  out.bits.pc := in.pc

  val sIDLE :: sWAIT :: Nil = Enum(2)

  val state = RegInit(sIDLE)
  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> sWAIT,
      sWAIT -> Mux(out.ready, sIDLE, sWAIT)
    )
  )
  fetch_port_out.ifu_addr := Mux(state === sIDLE, in.pc, 0.U(32.W))
  fetch_port_out.ifu_valid := state === sIDLE
  out.valid := state === sWAIT

}
