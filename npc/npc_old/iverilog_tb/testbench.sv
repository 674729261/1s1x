`timescale 1ns / 100ps
module tb_npc;
  reg clock, reset;
  reg [31:0] mem[0:32'h3FFFFFF];
  always #0.5 clock = ~clock;
  initial begin
    clock = 1'b1;
    reset = 1'b1;
  end

endmodule
