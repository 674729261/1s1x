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
    val ifu_respValid = Input(Bool())
    val instr = Input(UInt(32.W))
  })

  val fetch_port_out = IO(new Bundle {
    val ifu_addr = Output(UInt(32.W))
    val ifu_reqValid = Output(Bool())

  })

  val out = IO(DecoupledIO(new MessageIFU2IDU))
  out.bits.inst := in.instr
  out.bits.pc := in.pc

  val sIDLE :: sWAIT_RESP :: sWAIT_READY :: Nil = Enum(3)

  val state = RegInit(sIDLE)
  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> sWAIT_RESP,
      sWAIT_RESP -> Mux(out.ready, sIDLE, sWAIT_RESP)
    )
  )
  fetch_port_out.ifu_addr := Mux(state === sIDLE, in.pc, 0.U(32.W))
  fetch_port_out.ifu_reqValid := state === sIDLE
  out.valid := in.ifu_respValid && state === sWAIT_RESP

}
