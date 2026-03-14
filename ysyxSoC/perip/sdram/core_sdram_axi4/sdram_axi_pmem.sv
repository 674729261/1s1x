module sdram_axi_pmem (
      input        clk_i
    , input        rst_i
    , input        axi_awvalid_i
    , input [31:0] axi_awaddr_i
    , input [ 3:0] axi_awid_i
    , input [ 7:0] axi_awlen_i
    , input [ 1:0] axi_awburst_i
    , input        axi_wvalid_i
    , input [31:0] axi_wdata_i
    , input [ 3:0] axi_wstrb_i
    , input        axi_wlast_i
    , input        axi_bready_i
    , input        axi_arvalid_i
    , input [31:0] axi_araddr_i
    , input [ 3:0] axi_arid_i
    , input [ 7:0] axi_arlen_i
    , input [ 1:0] axi_arburst_i
    , input        axi_rready_i
    , input        ram_accept_i
    , input        ram_ack_i
    , input        ram_error_i
    , input [31:0] ram_read_data_i

    , output        axi_awready_o
    , output        axi_wready_o
    , output        axi_bvalid_o
    , output [ 1:0] axi_bresp_o
    , output [ 3:0] axi_bid_o
    , output        axi_arready_o
    , output        axi_rvalid_o
    , output [31:0] axi_rdata_o
    , output [ 1:0] axi_rresp_o
    , output [ 3:0] axi_rid_o
    , output        axi_rlast_o
    , output [ 3:0] ram_wr_o
    , output        ram_rd_o
    , output [ 7:0] ram_len_o
    , output [31:0] ram_addr_o
    , output [31:0] ram_write_data_o
);
  wire ar_fire, aw_fire, w_fire, r_fire, b_fire, r_fire_last;
  assign ar_fire = axi_arvalid_i && axi_arready_o;
  assign aw_fire = axi_awvalid_i && axi_awready_o;
  assign w_fire = axi_wvalid_i && axi_wready_o;
  assign r_fire = axi_rvalid_o && axi_rready_i;
  assign b_fire = axi_bvalid_o && axi_bready_i;
  assign r_fire_last = axi_rvalid_o && axi_rready_i && axi_rlast_o;

  reg has_ar, has_aw, has_w;

  always @(posedge clk_i or posedge rst_i) begin
    if (rst_i) begin
      has_ar <= 1'b0;
      has_aw <= 1'b0;
      has_w  <= 1'b0;
    end else begin
      if (ar_fire) has_ar <= 1'b1;
      else if (r_fire_last) has_ar <= 1'b0;

      if (aw_fire) has_aw <= 1'b1;
      else if (b_fire) has_aw <= 1'b0;

      if (w_fire) has_w <= 1'b1;
      else if (b_fire) has_w <= 1'b0;
    end
  end



  reg [3:0] axi_id;
  reg [2:0] read_cnt, burst_cnt, send_cnt;
  reg [31:0] addr;
  reg [31:0] wdata;
  reg [ 3:0] wstrb;
  // reg [3:0] state, nxt_state;
  // localparam ST_IDLE = 0;
  // localparam ST_RAM_ACCESS_READ = 1;
  // localparam ST_WAIT_READ = 2;
  // localparam ST_AXI_R = 3;
  // localparam ST_RAM_ACCESS_WRITE = 4;
  // localparam ST_AXI_B = 5;

  reg [31:0] read_buffer[0:7];
  reg [2:0] in_ptr, out_ptr;
  reg ram_out_last_r, ram_out_last_w;

  always_ff @(posedge clk_i) begin
    if (aw_fire) axi_id <= axi_awid_i;
    else if (ar_fire) axi_id <= axi_arid_i;

    if (ar_fire) read_cnt <= axi_arlen_i[2:0];
    else if (ram_ack_i) read_cnt <= read_cnt - 3'd1;

    if (ar_fire) send_cnt <= axi_arlen_i[2:0];
    else if (ram_accept_i) send_cnt <= send_cnt - 3'd1;

    if (ar_fire) burst_cnt <= axi_arlen_i[2:0];
    else if (r_fire) burst_cnt <= burst_cnt - 3'd1;

    if (ar_fire) addr <= axi_araddr_i;
    else if (aw_fire) addr <= axi_awaddr_i;
    else if (ram_accept_i) addr <= addr + 32'd4;

    if (w_fire) wstrb <= axi_wstrb_i;
    if (w_fire) wdata <= axi_wdata_i;

    if (ar_fire) in_ptr <= 3'd0;
    else if (ram_ack_i) in_ptr <= in_ptr + 3'd1;

    if (ar_fire) out_ptr <= 3'd0;
    else if (r_fire) out_ptr <= out_ptr + 3'd1;

    if (ar_fire) ram_out_last_r <= 1'b0;
    else if (send_cnt == 3'd0 && ram_accept_i) ram_out_last_r <= 1'b1;

    if ((aw_fire || has_aw) && (w_fire || has_w)) ram_out_last_w <= 1'b0;
    else if (ram_accept_i) ram_out_last_w <= 1'b1;

  end


  reg [3:0] data_cnt_r_remaining;
  always_ff @(posedge clk_i) begin
    if (ar_fire) data_cnt_r_remaining <= 4'd0;
    else if (ram_ack_i && !r_fire) data_cnt_r_remaining <= data_cnt_r_remaining + 4'd1;
    else if (!ram_ack_i && r_fire) data_cnt_r_remaining <= data_cnt_r_remaining - 4'd1;

  end



  always_ff @(posedge clk_i) begin
    if (ram_ack_i) read_buffer[in_ptr] <= ram_read_data_i;
  end



  // always_ff @(posedge clk_i or posedge rst_i) begin
  //   if (rst_i) state <= ST_IDLE;
  //   else state <= nxt_state;
  // end


  assign ram_addr_o = addr;
  assign ram_rd_o = (has_ar && !ram_out_last_r);
  assign ram_wr_o = ((has_aw && has_w) ? wstrb : 4'b0000);
  assign ram_len_o = {5'b0, read_cnt};
  assign ram_write_data_o = wdata;

  assign axi_rid_o = axi_id;
  assign axi_bid_o = axi_id;
  assign axi_rresp_o = 2'b00;
  assign axi_bresp_o = 2'b00;
  assign axi_rdata_o = read_buffer[out_ptr];
  assign axi_rlast_o = (burst_cnt == 3'd0);


  assign axi_arready_o = !has_ar && !has_aw && !has_w && !axi_awvalid_i && !axi_wvalid_i;
  assign axi_awready_o = !has_ar && !has_aw;
  assign axi_wready_o = !has_ar && !has_w;
  assign axi_rvalid_o = (has_ar && data_cnt_r_remaining != 4'd0);
  assign axi_bvalid_o = (has_aw && has_w);



  // always_comb begin
  //   case (state)
  //     ST_IDLE:
  //     if (ar_fire) nxt_state = ST_RAM_ACCESS_READ;
  //     else if ((aw_fire || has_aw) && (w_fire || has_w)) nxt_state = ST_RAM_ACCESS_WRITE;
  //     else nxt_state = ST_IDLE;
  //     ST_RAM_ACCESS_READ:
  //     if (send_cnt == 3'd0 && ram_accept_i) nxt_state = ST_WAIT_READ;
  //     else nxt_state = ST_RAM_ACCESS_READ;
  //     ST_WAIT_READ:
  //     if (read_cnt == 3'd0 && ram_ack_i) nxt_state = ST_AXI_R;
  //     else nxt_state = ST_WAIT_READ;
  //     ST_RAM_ACCESS_WRITE:
  //     if (ram_accept_i) nxt_state = ST_AXI_B;
  //     else nxt_state = ST_RAM_ACCESS_WRITE;
  //     ST_AXI_B:
  //     if (b_fire) nxt_state = ST_IDLE;
  //     else nxt_state = ST_AXI_B;
  //     ST_AXI_R:
  //     if (r_fire_last) nxt_state = ST_IDLE;
  //     else nxt_state = ST_AXI_R;
  //     default: nxt_state = ST_IDLE;
  //   endcase
  // end



  always @(posedge clk_i) begin
    if (!rst_i) begin
      if (ar_fire) begin
        if (axi_arburst_i != 2'b01) $display("SDRAM axi burst must be 2'b01");
        if (axi_arlen_i >= 8'd8) $display("SDRAM axi read burst length must be <= 8");
      end

      if (aw_fire) begin
        if (axi_awburst_i != 2'b01) $display("SDRAM axi burst must be 2'b01");
        if (axi_awlen_i >= 8'd1) $display("SDRAM axi burst write is not supported");
      end
    end
  end


`ifdef verilator
  // reg [255:0] dbg_state;

  // always @* begin
  //   case (state)
  //     ST_IDLE: dbg_state = "ST_IDLE";
  //     ST_RAM_ACCESS_READ: dbg_state = "ST_RAM_ACCESS_READ";
  //     ST_WAIT_READ: dbg_state = "ST_WAIT_READ";
  //     ST_RAM_ACCESS_WRITE: dbg_state = "ST_RAM_ACCESS_WRITE";
  //     ST_AXI_B: dbg_state = "ST_AXI_B";
  //     ST_AXI_R: dbg_state = "ST_AXI_R";
  //     default: dbg_state = "UNKNOWN";
  //   endcase
  // end
`endif
endmodule
