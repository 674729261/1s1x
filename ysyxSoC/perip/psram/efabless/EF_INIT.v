`timescale 1ns / 1ps  //
`default_nettype none
// Using 35H Command
module PSRAM_QPI_SETTER (
    input  wire clk,
    input  wire rst_n,
    input  wire fire,
    output wire done,

    output reg        sck,
    output reg        ce_n,
    input  wire [3:0] din,
    output wire [3:0] dout,
    output wire       douten
);
  //localparam  DATA_START = 14;
  localparam IDLE = 1'b0, WRITE = 1'b1;

  wire [7:0] FINAL_COUNT = 7;

  reg state, nstate;
  reg  [7:0] counter;
  //reg [7:0]   data [3:0];

  wire [7:0] CMD_35H = 8'h35;

  always @*
    case (state)
      IDLE:  if (fire) nstate = WRITE;
 else nstate = IDLE;
      WRITE: if (done) nstate = IDLE;
 else nstate = WRITE;
    endcase

  always @(posedge clk or negedge rst_n)
    if (!rst_n) state <= IDLE;
    else state <= nstate;

  // Drive the Serial Clock (sck) @ clk/2
  always @(posedge clk or negedge rst_n)
    if (!rst_n) sck <= 1'b0;
    else if (~ce_n) sck <= ~sck;
    else if (state == IDLE) sck <= 1'b0;

  // ce_n logic
  always @(posedge clk or negedge rst_n)
    if (!rst_n) ce_n <= 1'b1;
    else if (state == WRITE) ce_n <= 1'b0;
    else ce_n <= 1'b1;

  always @(posedge clk or negedge rst_n)
    if (!rst_n) counter <= 8'b0;
    else if (sck & ~done) counter <= counter + 1'b1;
    else if (state == IDLE) counter <= 8'b0;


  assign dout   = (counter < 8) ? {3'b0, CMD_35H[7-counter]} : 4'b0;

  assign douten = (~ce_n);

  assign done   = (counter == FINAL_COUNT + 1);


endmodule
