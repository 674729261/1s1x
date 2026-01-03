// package empty
// import chisel3._
// import chisel3.util._
// import chisel3.util.random.LFSR

// class __inst_fetch_bus() extends Module {
//   val fetch_port = IO(Flipped(new AXI))
//   set_flipped_AXIfull_zero(fetch_port)
//   val io = IO(new Bundle {
//     val addr = Output(UInt(32.W))
//     val valid = Output(Bool())
//     val instr = Input(UInt(32.W))
//   })
//   val sBUSY :: sWAIT_REQ :: sFETCH :: sWAIT_READY :: Nil = Enum(4)
//   val state_next = Wire(UInt(sBUSY.getWidth.W))
//   val state = RegEnable(state_next, sWAIT_REQ, true.B)

//   val random_delay = Cat(0.U(29.W), LFSR(3))
//   val counter_next = Wire(UInt(32.W))
//   val counter_delay = RegEnable(counter_next, 0.U(32.W), true.B)

//   state_next := MuxLookup(state, sWAIT_REQ)(
//     Seq(
//       sBUSY -> Mux(counter_delay === 0.U(32.W), sWAIT_REQ, sBUSY),
//       sWAIT_REQ -> Mux(fetch_port.arvalid, sFETCH, sWAIT_REQ),
//       sFETCH -> Mux(counter_delay === 0.U(32.W), sWAIT_READY, sFETCH),
//       sWAIT_READY -> Mux(fetch_port.rready, sBUSY, sWAIT_READY)
//     )
//   )

//   counter_next := Mux(
//     (state === sWAIT_READY && state_next === sBUSY) || (state === sWAIT_REQ && state_next === sFETCH),
//     random_delay,
//     Mux(
//       counter_delay === 0.U(32.W),
//       0.U(32.W),
//       counter_delay - 1.U(32.W)
//     )
//   )

//   fetch_port.arready := state === sWAIT_REQ
//   fetch_port.rvalid := state === sWAIT_READY

//   io.addr := fetch_port.araddr
//   io.valid := state === sWAIT_REQ && state_next === sFETCH
//   fetch_port.rdata := Mux(state === sWAIT_READY, io.instr, 0.U(32.W))

// }
