module top_e7 (
    clk,
    rst,
    ps2_clk,
    ps2_data,
    seg0,
    seg1,
    seg3,
    seg4,
    seg6,
    seg7,
    ctrl,
    shift
);
  input clk, rst, ps2_clk, ps2_data;

  wire [7:0] data, ascii, display, display_data;
  wire ready, ready_last, overflow, initial_nextdata;

  output [6:0] seg0, seg1, seg3, seg4, seg6, seg7;
  output ctrl, shift;


  Reg #(
      .WIDTH(1),
      .RESET_VAL(1'b1)
  ) u_reg_ready (
      .clk (clk),
      .rst (rst),
      .din (~ready),
      .dout(ready_last),
      .wen (1'b1)
  );
  wire read;
  assign read = ready & ~ready_last;

  ps2_keyboard u_ps2_keyboard (
      .clk       (clk),
      .clrn      (~rst),
      .ps2_clk   (ps2_clk),
      .ps2_data  (ps2_data),
      .data      (data),
      .ready     (ready),
      .nextdata_n(~(initial_nextdata | read)),
      .overflow  (overflow)
  );

  keyLUT u_keyLUT (
      .code (data),
      .ascii(ascii)
  );

  Reg #(
      .WIDTH(1),
      .RESET_VAL(1'b1)
  ) u_initial_nextdata (
      .clk (clk),
      .rst (rst),
      .din (1'b0),
      .dout(initial_nextdata),
      .wen (1'b1)
  );
  wire release_key = (data == 8'hf0);
  wire waiting_release;

  Reg #(
      .WIDTH(1)
  ) u_reg_release (
      .clk (clk),
      .rst (rst),
      .din (release_key),
      .dout(waiting_release),
      .wen (read)
  );

  Reg #(
      .WIDTH(8)
  ) u_reg_ascii (
      .clk (clk),
      .rst (rst),
      .din (release_key | waiting_release ? 8'h00 : ascii),
      .dout(display),
      .wen (read)
  );

  Reg #(
      .WIDTH(8)
  ) u_reg_code (
      .clk (clk),
      .rst (rst),
      .din (release_key | waiting_release ? 8'h00 : data),
      .dout(display_data),
      .wen (read)
  );

  reg [127:0] pressed;

  always @(posedge clk) begin
    if (rst) pressed <= 128'h0;
    else if (read)
      if (waiting_release) begin
        pressed[data[6:0]] <= 1'b0;
      end else if (~release_key) begin
        pressed[data[6:0]] <= 1'b1;
      end
  end

  wire [7:0] count;

  Reg #(
      .WIDTH(8)
  ) u_reg_pressed (
      .clk (clk),
      .rst (rst),
      .din (count + 8'd1),
      .dout(count),
      .wen (read & ~waiting_release & ~release_key & ~pressed[data[6:0]])
  );

  wire [6:0] seg0_, seg1_, seg3_, seg4_;

  bcd7seg u_bcd7seg2 (
      .b(display_data[3:0]),
      .h(seg0_)
  );
  assign seg0 = display_data == 8'h0 ? 7'h7f : seg0_;
  bcd7seg u_bcd7seg3 (
      .b(display_data[7:4]),
      .h(seg1_)
  );
  assign seg1 = display_data == 8'h0 ? 7'h7f : seg1_;
  bcd7seg u_bcd7seg0 (
      .b(display[3:0]),
      .h(seg3_)
  );
  assign seg3 = display == 8'h0 ? 7'h7f : seg3_;
  bcd7seg u_bcd7seg1 (
      .b(display[7:4]),
      .h(seg4_)
  );
  assign seg4 = display == 8'h0 ? 7'h7f : seg4_;

  bcd7seg u_bcd7seg7 (
      .b(count[3:0]),
      .h(seg6)
  );
  bcd7seg u_bcd7seg8 (
      .b(count[7:4]),
      .h(seg7)
  );

  assign ctrl  = ~pressed[7'h14];
  assign shift = ~pressed[7'h12];

endmodule

module keyLUT (
    code,
    ascii
);
  input [7:0] code;
  output reg [7:0] ascii;

  always @(*) begin
    case (code)
      8'h1c:   ascii = 8'h61;  // a
      8'h32:   ascii = 8'h62;  // b
      8'h21:   ascii = 8'h63;  // c
      8'h23:   ascii = 8'h64;  // d
      8'h24:   ascii = 8'h65;  // e
      8'h2b:   ascii = 8'h66;  // f
      8'h34:   ascii = 8'h67;  // g
      8'h33:   ascii = 8'h68;  // h
      8'h43:   ascii = 8'h69;  // i
      8'h3b:   ascii = 8'h6a;  // j
      8'h42:   ascii = 8'h6b;  // k
      8'h4b:   ascii = 8'h6c;  // l
      8'h3a:   ascii = 8'h6d;  // m
      8'h31:   ascii = 8'h6e;  // n
      8'h44:   ascii = 8'h6f;  // o
      8'h4d:   ascii = 8'h70;  // p
      8'h15:   ascii = 8'h71;  // q
      8'h2d:   ascii = 8'h72;  // r
      8'h1b:   ascii = 8'h73;  // s
      8'h2c:   ascii = 8'h74;  // t
      8'h3c:   ascii = 8'h75;  // u
      8'h2a:   ascii = 8'h76;  // v
      8'h1d:   ascii = 8'h77;  // w
      8'h22:   ascii = 8'h78;  // x
      8'h35:   ascii = 8'h79;  // y
      8'h1a:   ascii = 8'h7a;  // z
      8'h45:   ascii = 8'h30;  // 0
      8'h16:   ascii = 8'h31;  // 1
      8'h1e:   ascii = 8'h32;  // 2
      8'h26:   ascii = 8'h33;  // 3
      8'h25:   ascii = 8'h34;  // 4
      8'h2e:   ascii = 8'h35;  // 5
      8'h36:   ascii = 8'h36;  // 6
      8'h3d:   ascii = 8'h37;  // 7
      8'h3e:   ascii = 8'h38;  // 8
      8'h46:   ascii = 8'h39;  // 9
      default: ascii = 8'h00;
    endcase
  end

endmodule

module ps2_keyboard (
    clk,
    clrn,
    ps2_clk,
    ps2_data,
    data,
    ready,
    nextdata_n,
    overflow
);
  input clk, clrn, ps2_clk, ps2_data;
  input nextdata_n;
  output [7:0] data;
  output reg ready;
  output reg overflow;  // fifo overflow
  // internal signal, for test
  reg [9:0] buffer;  // ps2_data bits
  reg [7:0] fifo                     [7:0];  // data fifo
  reg [2:0] w_ptr, r_ptr;  // fifo write and read pointers
  reg [3:0] count;  // count ps2_data bits
  // detect falling edge of ps2_clk
  reg [2:0] ps2_clk_sync;

  always @(posedge clk) begin
    ps2_clk_sync <= {ps2_clk_sync[1:0], ps2_clk};
  end

  wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

  always @(posedge clk) begin
    if (clrn == 0) begin  // reset
      count <= 0;
      w_ptr <= 0;
      r_ptr <= 0;
      overflow <= 0;
      ready <= 0;
    end else begin
      if (ready) begin  // read to output next data
        if(nextdata_n == 1'b0) //read next data
                begin
          r_ptr <= r_ptr + 3'b1;
          if (w_ptr == (r_ptr + 1'b1))  //empty
            ready <= 1'b0;
        end
      end
      if (sampling) begin
        if (count == 4'd10) begin
          if ((buffer[0] == 0) &&  // start bit
              (ps2_data) &&  // stop bit
              (^buffer[9:1])) begin  // odd  parity
            fifo[w_ptr] <= buffer[8:1];  // kbd scan code
            w_ptr <= w_ptr + 3'b1;
            ready <= 1'b1;
            overflow <= overflow | (r_ptr == (w_ptr + 3'b1));
          end
          count <= 0;  // for next
        end else begin
          buffer[count] <= ps2_data;  // store ps2_data
          count <= count + 3'b1;
        end
      end
    end
  end
  assign data = fifo[r_ptr];  //always set output data

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
