// define this macro to enable fast behavior simulation
// for flash by skipping SPI transfers
// `define FAST_FLASH

module spi_top_apb #(
    parameter flash_addr_start = 32'h30000000,
    parameter flash_addr_end   = 32'h3fffffff,
    parameter spi_ss_num       = 8
) (
    input         clock,
    input         reset,
    input  [31:0] in_paddr,
    input         in_psel,
    input         in_penable,
    input  [ 2:0] in_pprot,
    input         in_pwrite,
    input  [31:0] in_pwdata,
    input  [ 3:0] in_pstrb,
    output        in_pready,
    output [31:0] in_prdata,
    output        in_pslverr,

    output                  spi_sck,
    output [spi_ss_num-1:0] spi_ss,
    output                  spi_mosi,
    input                   spi_miso,
    output                  spi_irq_out
);

`ifdef FAST_FLASH

  wire [31:0] data;
  parameter invalid_cmd = 8'h0;
  flash_cmd flash_cmd_i (
      .clock(clock),
      .valid(in_psel && !in_penable),
      .cmd  (in_pwrite ? invalid_cmd : 8'h03),
      .addr ({8'b0, in_paddr[23:2], 2'b0}),
      .data (data)
  );
  assign spi_sck     = 1'b0;
  assign spi_ss      = 8'b0;
  assign spi_mosi    = 1'b1;
  assign spi_irq_out = 1'b0;
  assign in_pslverr  = 1'b0;
  assign in_pready   = in_penable && in_psel && !in_pwrite;
  assign in_prdata   = data[31:0];

`else

  reg  [ 4:0] xip_paddr;
  reg         xip_psel;
  reg         xip_penable;
  reg         xip_pwrite;
  reg  [31:0] xip_pwdata;
  reg  [ 3:0] xip_pstrb;
  reg         xip_pready;
  reg  [31:0] xip_prdata;
  reg         xip_pslverr;
  reg         xip_spi_irq_out;


  wire [ 4:0] spi_paddr;
  wire        spi_psel;
  wire        spi_penable;
  wire        spi_pwrite;
  wire [31:0] spi_pwdata;
  wire [ 3:0] spi_pstrb;
  wire        spi_pready;
  wire [31:0] spi_prdata;
  wire        spi_pslverr;
  wire        spi_spi_irq_out;


  wire        is_xip;
  assign is_xip = in_paddr[31:28] == 4'h3;
  // state machine to perform XIP sequence when address[31:28] == 4'h3
  localparam S_IDLE = 4'd0;
  localparam S_WR14 = 4'd1;
  localparam S_WR18 = 4'd2;
  localparam S_WR10 = 4'd3;
  localparam S_POLL10 = 4'd4;
  localparam S_RD0 = 4'd5;
  localparam S_RD4 = 4'd6;
  localparam S_RESPOND = 4'd7;

  reg [ 3:0] state;
  reg [31:0] read0;
  reg [31:0] read4;
  reg [63:0] concat64;
  reg        seq_done;

  // default values
  always @(posedge clock) begin
    if (reset) begin
      xip_paddr       <= 5'd0;
      xip_psel        <= 1'b0;
      xip_penable     <= 1'b0;
      xip_pwrite      <= 1'b0;
      xip_pwdata      <= 32'd0;
      xip_pstrb       <= 4'b1111;
      xip_pready      <= 1'b0;
      xip_prdata      <= 32'd0;
      xip_pslverr     <= 1'b0;
      xip_spi_irq_out <= 1'b0;
      state           <= S_IDLE;
      read0           <= 32'd0;
      read4           <= 32'd0;
      concat64        <= 64'd0;
      seq_done        <= 1'b0;
    end else begin
      // default drive values each cycle (will be overwritten per-state)
      xip_pslverr <= 1'b0;
      xip_spi_irq_out <= 1'b0;

      case (state)
        S_IDLE: begin
          // capture request on APB setup phase (psel && !penable) in XIP region
          if (in_psel && !in_penable && is_xip) begin
            seq_done <= 1'b0;
            xip_pready <= 1'b0;
            // start first write to addr 0x14
            xip_paddr   <= 5'h14;
            xip_pwrite  <= 1'b1;
            xip_pwdata  <= 32'h00000001;
            xip_pstrb   <= 4'b1111;
            xip_psel    <= 1'b1;
            xip_penable <= 1'b1;
            state <= S_WR14;
          end else begin
            xip_psel <= 1'b0;
            xip_penable <= 1'b0;
            xip_pwrite <= 1'b0;
            xip_pwdata <= 32'd0;
            xip_paddr <= 5'd0;
          end
        end

        S_WR14: begin
          // wait for spi ack
          if (spi_pready) begin
            // deassert bus
            xip_psel <= 1'b0;
            xip_penable <= 1'b0;
            xip_pwrite <= 1'b0;
            // next write to 0x18
            xip_paddr   <= 5'h18;
            xip_pwrite  <= 1'b1;
            xip_pwdata  <= 32'h00000080;
            xip_pstrb   <= 4'b1111;
            xip_psel    <= 1'b1;
            xip_penable <= 1'b1;
            state <= S_WR18;
          end else begin
            // keep asserting until ack
            xip_psel <= 1'b1;
            xip_penable <= 1'b1;
          end
        end

        S_WR18: begin
          if (spi_pready) begin
            xip_psel <= 1'b0;
            xip_penable <= 1'b0;
            xip_pwrite <= 1'b0;
            // write 0x2740 to addr 0x10
            xip_paddr   <= 5'h10;
            xip_pwrite  <= 1'b1;
            xip_pwdata  <= 32'h00002740;
            xip_pstrb   <= 4'b1111;
            xip_psel    <= 1'b1;
            xip_penable <= 1'b1;
            state <= S_WR10;
          end else begin
            xip_psel <= 1'b1;
            xip_penable <= 1'b1;
          end
        end

        S_WR10: begin
          if (spi_pready) begin
            xip_psel <= 1'b0;
            xip_penable <= 1'b0;
            xip_pwrite <= 1'b0;
            // begin polling read of addr 0x10
            xip_paddr   <= 5'h10;
            xip_pwrite  <= 1'b0;
            xip_pwdata  <= 32'd0;
            xip_pstrb   <= 4'b0000;
            xip_psel    <= 1'b1;
            xip_penable <= 1'b1;
            state <= S_POLL10;
          end else begin
            xip_psel <= 1'b1;
            xip_penable <= 1'b1;
          end
        end

        S_POLL10: begin
          if (spi_pready) begin
            // spi_prdata contains read value
            if (spi_prdata == 32'h00002640) begin
              // stop polling and read addr 0x0 then 0x4
              xip_psel <= 1'b0;
              xip_penable <= 1'b0;
              xip_pwrite <= 1'b0;
              // read addr 0x0
              xip_paddr   <= 5'h00;
              xip_pwrite  <= 1'b0;
              xip_pwdata  <= 32'd0;
              xip_pstrb   <= 4'b0000;
              xip_psel    <= 1'b1;
              xip_penable <= 1'b1;
              state <= S_RD0;
            end else begin
              // keep polling: reissue read
              xip_psel <= 1'b1;
              xip_penable <= 1'b1;
              xip_pwrite <= 1'b0;
              xip_paddr <= 5'h10;
            end
          end else begin
            // hold bus asserted while waiting for ack
            xip_psel <= 1'b1;
            xip_penable <= 1'b1;
            xip_pwrite <= 1'b0;
            xip_paddr <= 5'h10;
          end
        end

        S_RD0: begin
          if (spi_pready) begin
            read0 <= spi_prdata;
            xip_psel <= 1'b0;
            xip_penable <= 1'b0;
            // now read addr 0x4
            xip_paddr   <= 5'h04;
            xip_pwrite  <= 1'b0;
            xip_pwdata  <= 32'd0;
            xip_pstrb   <= 4'b0000;
            xip_psel    <= 1'b1;
            xip_penable <= 1'b1;
            state <= S_RD4;
          end else begin
            xip_psel <= 1'b1;
            xip_penable <= 1'b1;
            xip_pwrite <= 1'b0;
            xip_paddr <= 5'h00;
          end
        end

        S_RD4: begin
          if (spi_pready) begin
            read4 <= spi_prdata;
            xip_psel <= 1'b0;
            xip_penable <= 1'b0;
            // combine and shift right by 1
            concat64 <= {spi_prdata, read0};
            seq_done <= 1'b1;
            state <= S_RESPOND;
          end else begin
            xip_psel <= 1'b1;
            xip_penable <= 1'b1;
            xip_pwrite <= 1'b0;
            xip_paddr <= 5'h04;
          end
        end

        S_RESPOND: begin
          // shift right by 1 and supply lower 32 bits on APB read response
          concat64   <= concat64;  // hold
          // perform the shift combinationally
          xip_prdata <= concat64[32:1];
          // assert pready to APB master when it is in enable phase and accessing XIP
          if (in_psel && in_penable && is_xip) begin
            xip_pready <= 1'b1;
          end else begin
            // once master sees the ready and completes transaction, clear and go idle
            if (xip_pready && !(in_psel && in_penable && is_xip)) begin
              xip_pready <= 1'b0;
              seq_done <= 1'b0;
              state <= S_IDLE;
            end
          end
        end

        default: begin
          state <= S_IDLE;
        end
      endcase
    end
  end


  assign spi_paddr   = is_xip ? xip_paddr   : in_paddr[4:0];
  assign spi_psel    = is_xip ? xip_psel    : in_psel;
  assign spi_penable = is_xip ? xip_penable : in_penable;
  assign spi_pwrite  = is_xip ? xip_pwrite  : in_pwrite;
  assign spi_pwdata  = is_xip ? xip_pwdata  : in_pwdata;
  assign spi_pstrb   = is_xip ? xip_pstrb   : in_pstrb;

  // route responses/irq back to APB master (use captured XIP regs when in_xip)
  assign in_pready   = is_xip ? xip_pready   : spi_pready;
  assign in_prdata   = is_xip ? xip_prdata   : spi_prdata;
  assign in_pslverr  = is_xip ? xip_pslverr  : spi_pslverr;
  assign spi_irq_out = is_xip ? xip_spi_irq_out : spi_spi_irq_out;




  spi_top u0_spi_top (
      .wb_clk_i(clock),
      .wb_rst_i(reset),
      .wb_adr_i(spi_paddr),
      .wb_dat_i(spi_pwdata),
      .wb_dat_o(spi_prdata),
      .wb_sel_i(spi_pstrb),
      .wb_we_i (spi_pwrite),
      .wb_stb_i(spi_psel),
      .wb_cyc_i(spi_penable),
      .wb_ack_o(spi_pready),
      .wb_err_o(spi_pslverr),
      .wb_int_o(spi_spi_irq_out),

      .ss_pad_o  (spi_ss),
      .sclk_pad_o(spi_sck),
      .mosi_pad_o(spi_mosi),
      .miso_pad_i(spi_miso)
  );

`endif  // FAST_FLASH

endmodule
