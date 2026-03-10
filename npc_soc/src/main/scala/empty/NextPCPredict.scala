package empty
import chisel3._
import chisel3.util._
import chisel3.layer._
import ujson.Arr

class BTBLine(tag_width: Int) extends Bundle {
  val tag = UInt(tag_width.W)
  val target = UInt(32.W)
}

class NextPCPredict(size_2pow: Int, tag_from: Int, tag_to: Int) extends Module {
  val io = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val predicted = Output(UInt(32.W))
    val write_pc = Input(UInt(32.W))
    val write_target = Input(UInt(32.W))
    val wen = Input(Bool())
  })
  val nr_lines = (1 << size_2pow)
  val tag_width = tag_from - tag_to + 1

  val input_tag = io.pc(tag_from, tag_to)
  val write_tag = io.write_pc(tag_from, tag_to)

  val content = Reg(Vec(4, new BTBLine(tag_width)))
  var compare_results = Wire(Vec(nr_lines, Bool()))
  for (i <- 0 until nr_lines) {
    compare_results(i) := (content(i).tag === input_tag)
  }
  var compare_results_write = Wire(Vec(nr_lines, Bool()))
  for (i <- 0 until nr_lines) {
    compare_results_write(i) := (content(i).tag === write_tag)
  }
  val read_hit = compare_results.reduce(_ || _)
  val write_hit = compare_results_write.reduce(_ || _)
  val onehot_seq_read = Array.ofDim[(Bool, UInt)](nr_lines)
  for (i <- 0 until nr_lines) {
    onehot_seq_read(i) = (compare_results(i) -> content(i).target)
  }
  val read_target = Mux1H(onehot_seq_read)
  io.predicted := Mux(read_hit, read_target, io.pc + 4.U)

  val write_ptr_nxt = Wire(UInt(size_2pow.W))
  val write_ptr =
    RegEnable(write_ptr_nxt, 0.U(size_2pow.W), io.wen && !write_hit)
  write_ptr_nxt := write_ptr + 1.U(size_2pow.W)

  when(io.wen) {
    when(write_hit) {
      for (i <- 0 until nr_lines) {
        when(compare_results_write(i)) { content(i).target := io.write_target }
      }
    }.otherwise {
      content(write_ptr).tag := write_tag
      content(write_ptr).target := io.write_target
    }
  }

  block(Verifying) {
    val compare_results_uint = compare_results.asUInt
    assert(
      (compare_results_uint & (compare_results_uint - 1.U)) === 0.U,
      "BTB contains more than 1 items match the input pc while reading"
    )
    val compare_results_uint_write = compare_results_write.asUInt
    assert(
      (compare_results_uint_write & (compare_results_uint_write - 1.U)) === 0.U,
      "BTB contains more than 1 items match the input pc while writing"
    )
  }
}
