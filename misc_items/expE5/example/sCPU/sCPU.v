module sCPU (
    clk,
    rst,
    en,
    seg0,
    seg1,
    out_valid,
    ins_addr,
    ins_data
);

  localparam ADD = 2'b00;
  localparam OUT = 2'b01;
  localparam LI = 2'b10;
  localparam BNE = 2'b11;


  input clk, rst, en;
  input [7:0] ins_data;
  output [3:0] ins_addr;
  output [6:0] seg0, seg1;
  output out_valid;

  wire [1:0] raddr1, raddr2, waddr;
  wire [7:0] rdata1, rdata2, wdata;
  wire wen;
  GPR sCPU_GPR (
      .clk   (clk),
      .rst   (rst),
      .raddr1(raddr1),
      .rdata1(rdata1),
      .raddr2(raddr2),
      .rdata2(rdata2),
      .waddr (waddr),
      .wdata (wdata),
      .wen   (wen & en)
  );



  wire [3:0] next_pc, pc;

  Reg #(
      .WIDTH(4)
  ) PC (
      .clk (clk),
      .rst (rst),
      .din (next_pc),
      .dout(pc),
      .wen (en & ~out_valid)
  );
  assign ins_addr = pc;
  wire [1:0] opcode, rs1, rs2, rd;
  wire [3:0] imm, jmp_addr;
  assign {opcode, rd, rs1, rs2} = ins_data;
  assign imm = ins_data[3:0];
  assign jmp_addr = ins_data[5:2];
  assign waddr = rd;
  assign wdata = opcode == ADD ? rdata1 + rdata2 : {4'h0, imm};
  assign wen = opcode == ADD || opcode == LI;


  assign raddr1 = opcode == BNE ? 2'b00 : rs1;
  assign raddr2 = rs2;
  assign next_pc = (opcode == BNE && rdata1 != rdata2) ? jmp_addr : pc + 4'h1;
  assign out_valid = opcode == OUT;

  bcd7seg out1 (
      .b(rdata2[3:0]),
      .h(seg0)
  );

  bcd7seg out2 (
      .b(rdata2[7:4]),
      .h(seg1)
  );


endmodule



module GPR (
    clk,
    rst,
    raddr1,
    raddr2,
    waddr,
    wdata,
    wen,
    rdata1,
    rdata2
);
  input [1:0] raddr1, raddr2, waddr;
  input wen, clk, rst;
  input [7:0] wdata;
  output [7:0] rdata1, rdata2;

  wire [7:0] GPRdata[0:3];
  genvar i;
  generate
    for (i = 0; i < 4; i = i + 1) begin
      Reg #(
          .WIDTH(8)
      ) register (
          .clk (clk),
          .rst (rst),
          .din (wdata),
          .dout(GPRdata[i]),
          .wen (waddr == i && wen)
      );
    end
  endgenerate

  assign rdata1 = GPRdata[raddr1];
  assign rdata2 = GPRdata[raddr2];

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
