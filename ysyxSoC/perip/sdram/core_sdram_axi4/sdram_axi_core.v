

module sdram_axi_core (
    // Inputs
      input        clk_i
    , input        rst_i
    , input [ 3:0] inport_wr_i
    , input        inport_rd_i
    , input [ 7:0] inport_len_i
    , input [31:0] inport_addr_i
    , input [31:0] inport_write_data_i
    , input [15:0] sdram_data_input0_low_i
    , input [15:0] sdram_data_input0_high_i
    , input [15:0] sdram_data_input1_low_i
    , input [15:0] sdram_data_input1_high_i

    // Outputs
    , output        inport_accept_o
    , output        inport_ack_o
    , output        inport_error_o
    , output [31:0] inport_read_data_o
    , output        sdram_clk_o
    , output        sdram_cke_o
    , output        sdram_cs_o
    , output        sdram_ras_o
    , output        sdram_cas_o
    , output        sdram_we_o
    , output [ 1:0] sdram_dqm0_low_o
    , output [ 1:0] sdram_dqm0_high_o
    , output [ 1:0] sdram_dqm1_low_o
    , output [ 1:0] sdram_dqm1_high_o

    , output [12:0] sdram_addr_o
    , output [ 1:0] sdram_ba_o
    , output [15:0] sdram_data_output0_low_o
    , output [15:0] sdram_data_output0_high_o
    , output [15:0] sdram_data_output1_low_o
    , output [15:0] sdram_data_output1_high_o
    , output        sdram_data_out_en_o
);

  parameter SDRAM_MHZ = 50;
  parameter SDRAM_ADDR_W = 24;
  parameter SDRAM_COL_W = 9;
  parameter SDRAM_READ_LATENCY = 'd1;

  localparam ST_INIT = 4'd0;
  localparam ST_MODE = 4'd1;
  localparam ST_IDLE = 4'd2;
  localparam ST_READ_ACTIVATE = 4'd3;
  localparam ST_READ = 4'd4;
  localparam ST_READ_WAIT = 4'd5;
  localparam ST_WRITE_ACTIVATE = 4'd6;
  localparam ST_WRITE = 4'd7;

  localparam CMD_W = 4;
  localparam CMD_NOP = 4'b0111;
  localparam CMD_ACTIVE = 4'b0011;
  localparam CMD_READ = 4'b0101;
  localparam CMD_WRITE = 4'b0100;
  localparam CMD_TERMINATE = 4'b0110;
  localparam CMD_PRECHARGE = 4'b0010;
  localparam CMD_REFRESH = 4'b0001;
  localparam CMD_LOAD_MODE = 4'b0000;

  localparam MODE_REG = {3'b000, 1'b0, 2'b00, 3'b010, 1'b0, 3'b000};

  localparam SDRAM_BANK_W = 2;
  localparam SDRAM_DQM_W = 2;
  localparam SDRAM_BANKS = 2 ** SDRAM_BANK_W;
  localparam SDRAM_ROW_W = SDRAM_ADDR_W - SDRAM_COL_W - SDRAM_BANK_W;

  reg [3:0] state, state_nxt;
  reg cke_q;

  always @(posedge clk_i or posedge rst_i) begin
    if (rst_i) begin
      state <= ST_INIT;
    end else begin
      state <= state_nxt;
    end
  end

  always @(posedge clk_i or posedge rst_i) begin
    if (rst_i) cke_q <= 1'b0;
    else cke_q <= 1'b1;
  end
  assign sdram_cke_o = cke_q;

  assign inport_error_o = 1'b0;

  wire in_req = inport_wr_i != 4'b0000 || inport_rd_i;

  wire ready_accept_r;



  assign inport_accept_o = in_req && ((inport_rd_i && (state == ST_READ_WAIT || (state == ST_READ && read_count != 8'd0))) || (state == ST_WRITE && inport_wr_i != 4'b0));
  wire req_fire = in_req && inport_accept_o;

  wire which_chip = inport_addr_i[SDRAM_ADDR_W+1+1];
  reg  which_chip_r;

  always @(posedge clk_i) begin
    if (state == ST_READ_ACTIVATE) which_chip_r <= which_chip;
  end

  // Address bits
  wire [SDRAM_ROW_W-1:0] addr_col_w = {
    {(SDRAM_ROW_W - SDRAM_COL_W) {1'b0}}, inport_addr_i[SDRAM_COL_W+1:2]
  };
  wire [SDRAM_ROW_W-1:0] addr_row_w = inport_addr_i[SDRAM_ADDR_W+1:SDRAM_COL_W+2+1+1];
  wire [SDRAM_BANK_W-1:0] addr_bank_w = inport_addr_i[SDRAM_COL_W+2+1:SDRAM_COL_W+2-1+1];

  reg [31:0] wait_count;
  reg [7:0] read_count;
  always @(posedge clk_i) begin
    if (state == ST_IDLE) wait_count <= 'd0;
    else if (state == ST_READ_ACTIVATE) wait_count <= SDRAM_READ_LATENCY;
    else if (state == ST_READ_WAIT) wait_count <= wait_count - 'd1;
  end
  always @(posedge clk_i) begin
    if (state == ST_IDLE) read_count <= inport_len_i;
    else if (state == ST_READ) read_count <= read_count - 'd1;
  end


  always @(*) begin
    state_nxt = state;
    case (state)
      ST_INIT: state_nxt = ST_MODE;
      ST_MODE: state_nxt = ST_IDLE;
      ST_IDLE:
      if (inport_rd_i) state_nxt = ST_READ_ACTIVATE;
      else if (inport_wr_i != 4'b0000) state_nxt = ST_WRITE_ACTIVATE;
      else state_nxt = ST_IDLE;
      ST_READ_ACTIVATE: state_nxt = ST_READ_WAIT;
      ST_READ_WAIT: state_nxt = (wait_count == 'd0) ? ST_READ : ST_READ_WAIT;
      ST_READ: state_nxt = (read_count == 'd0) ? ST_IDLE : ST_READ;
      ST_WRITE_ACTIVATE: state_nxt = ST_WRITE;
      ST_WRITE: state_nxt = ST_IDLE;
      default: state_nxt = state;
    endcase
  end

  reg [3:0] command_q;
  assign sdram_cs_o  = command_q[3];
  assign sdram_ras_o = command_q[2];
  assign sdram_cas_o = command_q[1];
  assign sdram_we_o  = command_q[0];

  always @(*) begin
    if (state == ST_MODE) command_q = CMD_LOAD_MODE;
    else if (state == ST_READ_WAIT || state == ST_READ)
      command_q = (inport_rd_i && req_fire) ? CMD_READ : CMD_NOP;
    else if (state == ST_WRITE) command_q = CMD_WRITE;
    else if (state == ST_READ_ACTIVATE || state == ST_WRITE_ACTIVATE) command_q = CMD_ACTIVE;
    else command_q = CMD_NOP;

  end

  assign sdram_addr_o = (state == ST_MODE) ? MODE_REG : (state == ST_READ_ACTIVATE || state == ST_WRITE_ACTIVATE ? addr_row_w : addr_col_w);

  wire [3:0] msk_r = ~inport_wr_i;
  assign sdram_dqm0_low_o = which_chip ? 2'b11 : msk_r[1:0];
  assign sdram_dqm0_high_o = which_chip ? 2'b11 : msk_r[3:2];
  assign sdram_dqm1_low_o = which_chip ? msk_r[1:0] : 2'b11;
  assign sdram_dqm1_high_o = which_chip ? msk_r[3:2] : 2'b11;
  assign sdram_data_out_en_o = state == ST_WRITE;
  assign sdram_data_output0_low_o = inport_write_data_i[15:0];
  assign sdram_data_output0_high_o = inport_write_data_i[31:16];
  assign sdram_data_output1_low_o = inport_write_data_i[15:0];
  assign sdram_data_output1_high_o = inport_write_data_i[31:16];

  assign sdram_ba_o = addr_bank_w;
  assign sdram_clk_o = ~clk_i;

  assign inport_read_data_o = which_chip_r ? {sdram_data_input1_high_i, sdram_data_input1_low_i} : {sdram_data_input0_high_i, sdram_data_input0_low_i};


  assign inport_ack_o = state == ST_READ || state == ST_WRITE;

`ifdef verilator
  reg [79:0] dbg_state;

  always @* begin
    case (state)
      ST_INIT: dbg_state = "INIT";
      ST_MODE: dbg_state = "MODE";
      ST_IDLE: dbg_state = "IDLE";
      ST_READ_ACTIVATE: dbg_state = "READ_ACT";
      ST_READ: dbg_state = "READ";
      ST_READ_WAIT: dbg_state = "READ_WAIT";
      ST_WRITE_ACTIVATE: dbg_state = "WRITE_ACT";
      ST_WRITE: dbg_state = "WRITE";
      default: dbg_state = "UNKNOWN";
    endcase
  end
`endif

endmodule
