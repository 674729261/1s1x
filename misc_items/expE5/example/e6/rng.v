module top_e6 (
    clk,
    rst,
    load,
    initial_value,
    seg1,
    seg2,
    out
);
  input clk;
  input rst;
  input load;
  input [7:0] initial_value;
  output [6:0] seg1, seg2;

  output [7:0] out;

  rng #(
      .BITWIDTH(8),
      .XORWIDTH(4)
  ) u_rng (
      .clk          (clk),
      .rst          (rst),
      .load         (load),
      .initial_value(initial_value),
      .out          (out)
  );

  bcd7seg u_bcd7seg1 (
      .b(out[3:0]),
      .h(seg1)
  );
  bcd7seg u_bcd7seg2 (
      .b(out[7:4]),
      .h(seg2)
  );
endmodule

module rng #(
    BITWIDTH = 8,
    XORWIDTH = 4
) (
    clk,
    rst,
    load,
    initial_value,
    out
);
  input clk, rst, load;
  input [BITWIDTH-1:0] initial_value;

  output [BITWIDTH-1:0] out;

  Reg #(
      .WIDTH(BITWIDTH)
  ) u_reg (
      .clk (clk),
      .rst (rst),
      .din (load ? initial_value : {^{out[XORWIDTH-1:0]}, out[BITWIDTH-1:1]}),
      .dout(out),
      .wen (1'b1)
  );


endmodule

module Reg #(
    WIDTH = 1,
    RESET_VAL = 0
) (
    input clk,
    input rst,
    input [WIDTH-1:0] din,
    output reg [WIDTH-1:0] dout,
    input wen
);
  always @(posedge clk) begin
    if (rst) dout <= RESET_VAL;
    else if (wen) dout <= din;
  end
endmodule

module bcd7seg (
    input [3:0] b,
    output reg [6:0] h
);
  always @(*) begin
    case (b)
      4'd0: h = 7'b0000001;
      4'd1: h = 7'b1001111;
      4'd2: h = 7'b0010010;
      4'd3: h = 7'b0000110;
      4'd4: h = 7'b1001100;
      4'd5: h = 7'b0100100;
      4'd6: h = 7'b0100000;
      4'd7: h = 7'b0001111;
      4'd8: h = 7'b0000000;
      4'd9: h = 7'b0000100;
      4'd10: h = 7'b0001000;  // A
      4'd11: h = 7'b1100000;  // B
      4'd12: h = 7'b0110001;  // C
      4'd13: h = 7'b1000010;  // D
      4'd14: h = 7'b0110000;  // E
      4'd15: h = 7'b0111000;  // F
      default: h = 7'b1111111;
    endcase
  end

endmodule
