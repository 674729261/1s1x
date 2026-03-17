`timescale 1ns / 100ps
module tb_npc;
  reg clock, reset;
  reg [31:0] mem_init[0:2];

  reg [31:0] mem[0:32'h1FFFFFF];
  always #0.5 clock = ~clock;

  wire [31:0] raddr;
  wire [31:0] waddr;
  wire [31:0] wdata;
  wire [3:0] wmask;
  wire [31:0] wmask32;
  wire wen;
  wire valid;
  reg [31:0] rdata;
  integer inst_cnt;
  initial begin : init
    mem_init[0] = 32'h800002b7;
    mem_init[1] = 32'h00028067;
    $readmemh("../build/iv/temp.hex", mem);
    for (integer i = 0; i < 32'h2FFFF; i = i + 1) begin : convert_endian
      reg [31:0] raw;
      raw = mem[i];
      mem[i] = {raw[7-:8], raw[15-:8], raw[23-:8], raw[31-:8]};
    end
    $display("First 8 instructions :");
    $display("%08x %08x %08x %08x", mem[0], mem[1], mem[2], mem[3]);
    $display("%08x %08x %08x %08x", mem[4], mem[5], mem[6], mem[7]);

    inst_cnt = 0;
    clock = 1'b1;
    reset = 1'b1;
    repeat (3) @(negedge clock);
    reset = 1'b0;
  end
  ysyx_25080216 dut (
      .clock(clock),
      .reset(reset),

      .io_master_awready(u_simbus.awready),
      .io_master_awvalid(u_simbus.awvalid),
      .io_master_awaddr (u_simbus.awaddr),
      .io_master_awid   (u_simbus.awid),
      .io_master_awlen  (u_simbus.awlen),
      .io_master_awsize (u_simbus.awsize),
      .io_master_awburst(u_simbus.awburst),

      .io_master_wready(u_simbus.wready),
      .io_master_wvalid(u_simbus.wvalid),
      .io_master_wdata (u_simbus.wdata),
      .io_master_wstrb (u_simbus.wstrb),
      .io_master_wlast (u_simbus.wlast),

      .io_master_bready(u_simbus.bready),
      .io_master_bvalid(u_simbus.bvalid),
      .io_master_bresp (u_simbus.bresp),
      .io_master_bid   (u_simbus.bid),

      .io_master_arready(u_simbus.arready),
      .io_master_arvalid(u_simbus.arvalid),
      .io_master_araddr (u_simbus.araddr),
      .io_master_arid   (u_simbus.arid),
      .io_master_arlen  (u_simbus.arlen),
      .io_master_arsize (u_simbus.arsize),
      .io_master_arburst(u_simbus.arburst),

      .io_master_rready(u_simbus.rready),
      .io_master_rvalid(u_simbus.rvalid),
      .io_master_rresp (u_simbus.rresp),
      .io_master_rdata (u_simbus.rdata),
      .io_master_rlast (u_simbus.rlast),
      .io_master_rid   (u_simbus.rid)
  );

  __sim_bus u_simbus (
      .clock(clock),
      .reset(reset),

      .io_raddr(raddr),
      .io_waddr(waddr),
      .io_wdata(wdata),
      .io_rdata(rdata),
      .io_wmask(wmask),
      .io_valid(valid),
      .io_wen  (wen),

      .fetch_port_aw_ready(u_simbus.awready),
      .fetch_port_aw_valid(u_simbus.awvalid),
      .fetch_port_aw_addr (u_simbus.awaddr),
      .fetch_port_aw_id   (u_simbus.awid),
      .fetch_port_aw_len  (u_simbus.awlen),
      .fetch_port_aw_size (u_simbus.awsize),
      .fetch_port_aw_burst(u_simbus.awburst),

      .fetch_port_w_ready(u_simbus.wready),
      .fetch_port_w_valid(u_simbus.wvalid),
      .fetch_port_w_data (u_simbus.wdata),
      .fetch_port_w_strb (u_simbus.wstrb),
      .fetch_port_w_last (u_simbus.wlast),

      .fetch_port_b_ready(u_simbus.bready),
      .fetch_port_b_valid(u_simbus.bvalid),
      .fetch_port_b_resp (u_simbus.bresp),
      .fetch_port_b_id   (u_simbus.bid),

      .fetch_port_a_rready(u_simbus.arready),
      .fetch_port_a_rvalid(u_simbus.arvalid),
      .fetch_port_a_raddr (u_simbus.araddr),
      .fetch_port_a_rid   (u_simbus.arid),
      .fetch_port_a_rlen  (u_simbus.arlen),
      .fetch_port_a_rsize (u_simbus.arsize),
      .fetch_port_a_rburst(u_simbus.arburst),

      .fetch_port_r_ready(u_simbus.rready),
      .fetch_port_r_valid(u_simbus.rvalid),
      .fetch_port_r_resp (u_simbus.rresp),
      .fetch_port_r_data (u_simbus.rdata),
      .fetch_port_r_last (u_simbus.rlast),
      .fetch_port_r_id   (u_simbus.rid)
  );


  assign wmask32 = {{8{wmask[3]}}, {8{wmask[2]}}, {8{wmask[1]}}, {8{wmask[0]}}};

  always @(posedge clock) begin
    if (valid && !reset) begin
      if (raddr >= 32'h80000000) rdata <= mem[(raddr-32'h80000000)>>2];
      else rdata <= mem_init[(raddr-32'h30000000)>>2];
    end
  end

  always @(posedge clock) begin
    if (wen && !reset) begin
      if (waddr < 32'ha0000000)
        mem[(waddr-32'h80000000)>>2] = (mem[(waddr-32'h80000000)>>2] & ~wmask32) | (wdata & wmask32);
      else if (waddr == 32'ha00003f8) $fwrite("%c", wdata[7:0]);
    end
  end

  // always @(posedge clock) begin
  //   if (reset) inst_cnt <= 0;
  //   else if (step) begin
  //     inst_cnt = inst_cnt + 1;
  //     //   $display("PC = %08x", pc);
  //   end
  // end

  always @(posedge clock) begin
    if (ebreak && !reset) begin
      $display("HALT@PC=%08h, inst count : %0d", pc, inst_cnt);
      $display("a0 = %08h", dut.cpu.gpr.register_bank_regs_9_r);
      if (dut.cpu.gpr.register_bank_regs_9_r === 32'h0) begin
        $display("HIT GOOD TRAP");
        $finish;
      end else begin
        $display("HIT BAD TRAP");
        $fatal;
      end

    end
  end





endmodule
