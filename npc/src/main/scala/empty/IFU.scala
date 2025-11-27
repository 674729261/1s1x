package empty.empty
import chisel3._
import chisel3.util._
import empty.InstBus

class MessageIFU2IDU extends Bundle {
  val pc = (UInt(32.W))
  val inst = (UInt(32.W))
}

class IFU() extends Module with RequireAsyncReset {
  val in = IO(new Bundle {
    val pc = Input(UInt(32.W))
  })

  val fetch_port = IO(new InstBus)

  val out = IO(DecoupledIO(new MessageIFU2IDU))
  out.bits.inst := fetch_port.instr
  out.bits.pc := in.pc

  val sIDLE :: sWAIT :: Nil = Enum(2)

  val state = RegInit(sIDLE)
  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(fetch_port.reqReady, sWAIT, sIDLE),
      sWAIT -> Mux(out.ready, sIDLE, sWAIT)
    )
  )
  fetch_port.ifu_addr := Mux(state === sIDLE, in.pc, 0.U(32.W))
  fetch_port.reqValid := state === sIDLE
  fetch_port.respReady := fetch_port.respValid
  out.valid := fetch_port.respValid && state === sWAIT

}
