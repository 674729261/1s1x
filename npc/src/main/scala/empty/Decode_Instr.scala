package empty

import chisel3._
import chisel3.util._

class IType extends Bundle {
  val imm12 = UInt(12.W)
  val rs = UInt(5.W)
  val funct3 = UInt(3.W)
  val rd = UInt(5.W)
  val opcode = UInt(7.W)
}
