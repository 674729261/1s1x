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

class RType extends Bundle {
  val funct7 = UInt(7.W)
  val rs2 = UInt(5.W)
  val rs1 = UInt(5.W)
  val funct3 = UInt(3.W)
  val rd = UInt(5.W)
  val opcode = UInt(7.W)
}

class UType extends Bundle {
  val imm20 = UInt(20.W)
  val rd = UInt(5.W)
  val opcode = UInt(7.W)
}
