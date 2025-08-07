module Reg #(
    WIDTH = 1,
    RESET_VAL = 0
) (
    input                  clk,
    input                  rst,
    input      [WIDTH-1:0] din,
    output reg [WIDTH-1:0] dout,
    input                  wen
);
  always @(posedge clk) begin
    if (rst) dout <= RESET_VAL;
    else if (wen) dout <= din;
  end
endmodule

module Reg_n #(
    WIDTH = 1,
    RESET_VAL = 0
) (
    input                  clk,
    input                  rst,
    input      [WIDTH-1:0] din,
    output reg [WIDTH-1:0] dout,
    input                  wen
);
  always @(negedge clk) begin
    if (rst) dout <= RESET_VAL;
    else if (wen) dout <= din;
  end
endmodule



module test (
    input  clk,
    input  in,
    output out
);
  wire t0, t1;
  Reg r1 (
      clk,
      1'b0,
      in,
      t0,
      1'b1
  );
  Reg_n r2 (
      clk,
      1'b0,
      t0,
      t1,
      1'b1
  );
  Reg r3 (
      clk,
      1'b0,
      t1,
      out,
      1'b1
  );
endmodule
