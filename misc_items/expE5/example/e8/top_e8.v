module top_e8 (
    clk,
    rst,
    pause,
    VGA_HSYNC,
    VGA_VSYNC,
    VGA_BLANK_N,
    VGA_R,
    VGA_G,
    VGA_B
);

  input clk, rst;


  wire [ 9:0] h_addr;
  wire [ 9:0] v_addr;
  wire [23:0] vga_data;
  output VGA_HSYNC;
  output VGA_VSYNC;
  output VGA_BLANK_N;
  output [7:0] VGA_R;
  output [7:0] VGA_G;
  output [7:0] VGA_B;



  input pause;
  wire freq_out;

  get_data u_getdata (
      .h_addr(h_addr),
      .v_addr(v_addr),
      .update(h_addr == 10'd639 && v_addr == 10'd479),
      .data  (vga_data),
      .clk   (clk),
      .rst   (rst),
      .pause (pause)
  );


  vga_ctrl my_vga_ctrl (
      .pclk    (clk),
      .reset   (rst),
      .vga_data(vga_data),
      .h_addr  (h_addr),
      .v_addr  (v_addr),
      .hsync   (VGA_HSYNC),
      .vsync   (VGA_VSYNC),
      .valid   (VGA_BLANK_N),
      .vga_r   (VGA_R),
      .vga_g   (VGA_G),
      .vga_b   (VGA_B)
  );

endmodule

module get_data (
    h_addr,
    v_addr,
    update,
    data,
    clk,
    rst,
    pause
);

  input [9:0] h_addr, v_addr;
  input update, clk, rst, pause;
  output [23:0] data;
  reg [23:0] vga_mem[524287:0];
  initial begin
    $readmemh("picture.hex", vga_mem);
  end

  wire [10:0] offsetx, offsety;

  Reg #(
      .WIDTH(11)
  ) u_offsetx (
      .clk (clk),
      .rst (rst | offsetx == 11'd639),
      .din (offsetx + 10'd1),
      .dout(offsetx),
      .wen (update && ~pause)
  );

  Reg #(
      .WIDTH(11)
  ) u_offsety (
      .clk (clk),
      .rst (rst | offsety == 11'd479),
      .din (offsety + 10'd1),
      .dout(offsety),
      .wen (update && ~pause)
  );
  wire [10:0] add_h, add_v, h_off, v_off;
  assign add_h = {1'b0, h_addr} + offsetx;
  assign add_v = {1'b0, v_addr} + offsety;
  assign h_off = add_h >= 11'd640 ? add_h - 11'd640 : add_h;
  assign v_off = add_v >= 11'd480 ? add_v - 11'd480 : add_v;
  assign data  = vga_mem[{h_off[9:0], v_off[8:0]}];
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


module vga_ctrl (
    input pclk,
    input reset,
    input [23:0] vga_data,
    output [9:0] h_addr,
    output [9:0] v_addr,
    output hsync,
    output vsync,
    output valid,
    output [7:0] vga_r,
    output [7:0] vga_g,
    output [7:0] vga_b
);

  parameter h_frontporch = 96;
  parameter h_active = 144;
  parameter h_backporch = 784;
  parameter h_total = 800;

  parameter v_frontporch = 2;
  parameter v_active = 35;
  parameter v_backporch = 515;
  parameter v_total = 525;

  reg  [9:0] x_cnt;
  reg  [9:0] y_cnt;
  wire       h_valid;
  wire       v_valid;

  always @(posedge pclk) begin
    if (reset == 1'b1) begin
      x_cnt <= 1;
      y_cnt <= 1;
    end else begin
      if (x_cnt == h_total) begin
        x_cnt <= 1;
        if (y_cnt == v_total) y_cnt <= 1;
        else y_cnt <= y_cnt + 1;
      end else x_cnt <= x_cnt + 1;
    end
  end

  //生成同步信号    
  assign hsync = (x_cnt > h_frontporch);
  assign vsync = (y_cnt > v_frontporch);
  //生成消隐信号
  assign h_valid = (x_cnt > h_active) & (x_cnt <= h_backporch);
  assign v_valid = (y_cnt > v_active) & (y_cnt <= v_backporch);
  assign valid = h_valid & v_valid;
  //计算当前有效像素坐标
  assign h_addr = h_valid ? (x_cnt - 10'd145) : 10'd0;
  assign v_addr = v_valid ? (y_cnt - 10'd36) : 10'd0;
  //设置输出的颜色值
  assign {vga_r, vga_g, vga_b} = vga_data;

endmodule


