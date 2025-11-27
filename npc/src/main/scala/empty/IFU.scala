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

  val sIDLE :: sWAIT_RESP :: sWAIT :: Nil = Enum(3)

  val state = RegInit(sIDLE)
  state := MuxLookup(state, sIDLE)(
    Seq(
      sIDLE -> Mux(fetch_port.reqReady, sWAIT_RESP, sIDLE),
      sWAIT_RESP -> Mux(fetch_port.respValid, sWAIT, sWAIT_RESP),
      sWAIT -> Mux(out.ready, sIDLE, sWAIT)
    )
  )

  val inst_reg =
    RegEnable(fetch_port.instr, state === sWAIT_RESP && fetch_port.respValid)

  fetch_port.ifu_addr := Mux(state === sIDLE, in.pc, 0.U(32.W))
  fetch_port.reqValid := state === sIDLE
  fetch_port.respReady := state === sWAIT_RESP
  out.valid := state === sWAIT

  out.bits.inst := inst_reg
  out.bits.pc := in.pc

}
