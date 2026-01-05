module bitrev (
    input  sck,
    input  ss,
    input  mosi,
    output miso
);
  reg [7:0] rx_byte;
  always @(posedge sck) begin

  end
  assign miso = 1'b1;
endmodule
