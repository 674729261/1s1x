module Ebreaker (
    input clock,
    input ebreak
);
  always @(posedge clock) begin
    if (ebreak) $finish;
  end
endmodule
