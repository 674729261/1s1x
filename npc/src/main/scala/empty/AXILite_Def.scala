package empty

import chisel3._
import chisel3.util._

class ReadAddressChannel extends Bundle {
  val araddr = Output(UInt(32.W))
  val arvalid = Output(Bool())
  val arready = Input(Bool())
}
class ReadDataChannel extends Bundle {
  val rdata = Input(UInt(32.W))
  val rresp = Input(Bool())
  val rvalid = Input(Bool())
  val rready = Output(Bool())
}
class WriteDataChannel extends Bundle {
  val wdata = Output(UInt(32.W))
  val wstrb = Output(UInt(4.W))
  val wvalid = Output(Bool())
  val wready = Input(Bool())
}
class WriteAddressChannel extends Bundle {
  val awaddr = Output(UInt(32.W))
  val awvalid = Output(Bool())
  val awready = Input(Bool())
}
class RespChannel extends Bundle {
  val bresp = Input(UInt(2.W))
  val bvalid = Input(Bool())
  val bready = Output(Bool())
}

class AXI_Lite extends Bundle {
  val ar = new ReadAddressChannel
  val r = new ReadDataChannel
  val aw = new WriteAddressChannel
  val w = new WriteDataChannel
  val b = new RespChannel
}
