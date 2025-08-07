module top_e4 (
    clk,
    rst,
    pause_b,
    clear,
    stopwatch,
    dig0,
    dig1,
    dig2,
    dig3,
    dig4,
    dig5,
    dig6,
    dig7,

    left,
    right,
    up,
    down,
    confirm,

    tunemode
);
  input clk, rst, pause_b, stopwatch, clear;
  input left, right, up, down, confirm;
  output [6:0] dig0, dig1, dig2, dig3, dig4, dig5, dig6, dig7;
  output tunemode;
  wire [4:0] arrow, arrow_last;
  assign arrow = {left, right, up, down, confirm};

  Reg #(
      .WIDTH(5)
  ) u_reg_find_edge (
      .clk (clk),
      .rst (rst),
      .din (arrow),
      .dout(arrow_last),
      .wen (1'b1)
  );


  wire [4:0] arrow_edge;
  assign arrow_edge = arrow & ~arrow_last;
  wire tunemode;

  Reg #(
      .WIDTH(1)
  ) u_reg_tunemode (
      .clk (clk),
      .rst (rst),
      .din (stopwatch ? 1'b0 : (arrow_edge[0] ? ~tunemode : tunemode)),
      .dout(tunemode),
      .wen (~stopwatch)
  );

  wire [2:0] select;
  reg  [2:0] select_next;
  Reg #(
      .WIDTH(3)
  ) u_reg_select (
      .clk (clk),
      .rst (rst | ~tunemode),
      .din (select_next),
      .dout(select),
      .wen (1'b1)
  );

  always @(*) begin
    if (arrow_edge[4]) begin
      if (select == 3'd5) select_next = 3'd0;
      else select_next = select + 3'd1;
    end else if (arrow_edge[3]) begin
      if (select == 3'd0) select_next = 3'd5;
      else select_next = select - 3'd1;
    end else begin
      select_next = select;
    end
  end



  wire update, update1000hz, pause;
  assign pause = ~pause_b;
  get_freq u_get1hz (
      .clk  (clk),
      .rst  (rst),
      .pause(1'b0),
      .out  (update)
  );

  get_freq #(
      .TICKS(500)
  ) u_get1000hz (
      .clk  (clk),
      .rst  (rst | ~stopwatch),
      .pause(pause),
      .out  (update1000hz)
  );

  wire [3:0] second_low, minute_low;
  wire [2:0] second_high, minute_high;
  wire [3:0] hour_low;
  wire [1:0] hour_high;
  wire second_low_rst, second_high_rst, minute_low_rst, minute_high_rst,hour_low_rst, hour_high_rst;
  assign second_low_rst = update & second_low == 4'd9;
  assign second_high_rst = second_high == 3'd5 & second_low_rst;
  assign minute_low_rst = minute_low == 4'd9 & second_high_rst;
  assign minute_high_rst = minute_high == 3'd5 & minute_low_rst;
  assign hour_low_rst = (hour_low == 4'd9 || (hour_high == 2'd2 && hour_low == 4'd3)) & minute_high_rst;
  assign hour_high_rst = hour_high == 2'd2 & hour_low_rst;

  CounterAdd #(
      .WIDTH(4)
  ) u_counter_sec_low (
      .clk(clk),
      .rst  (rst | (second_low_rst && ~tunemode) | (tunemode & second_low == 4'd9 & (arrow_edge[1] | arrow_edge[2]) & select == 3'd0)),
      .en((update & ~tunemode) | (tunemode & (arrow_edge[1] | arrow_edge[2]) & select == 3'd0)),
      .count(second_low)
  );

  CounterAdd #(
      .WIDTH(3)
  ) u_counter_sec_high (
      .clk(clk),
      .rst(rst | (second_high_rst&& ~tunemode) | (tunemode & second_high == 3'd5 & (arrow_edge[1] | arrow_edge[2]) & select == 3'd1)),
      .en   ((update & second_low_rst& ~tunemode)  | (tunemode & (arrow_edge[1] | arrow_edge[2]) & select == 3'd1)),
      .count(second_high)
  );

  CounterAdd #(
      .WIDTH(4)
  ) u_counter_min_low (
      .clk(clk),
      .rst(rst | (minute_low_rst && ~tunemode) | (tunemode & minute_low == 4'd9 & (arrow_edge[1] | arrow_edge[2]) & select == 3'd2)),
      .en((update & second_high_rst& ~tunemode) | (tunemode & (arrow_edge[1] | arrow_edge[2]) & select == 3'd2)),
      .count(minute_low)
  );

  CounterAdd #(
      .WIDTH(3)
  ) u_counter_min_high (
      .clk(clk),
      .rst(rst | (minute_high_rst && ~tunemode) | (tunemode & minute_high == 3'd5 & (arrow_edge[1] | arrow_edge[2]) & select == 3'd3)),
      .en   ((update & minute_low_rst& ~tunemode) | (tunemode & (arrow_edge[1] | arrow_edge[2]) & select == 3'd3)),
      .count(minute_high)
  );

  CounterAdd #(
      .WIDTH(4)
  ) u_counter_hour_low (
      .clk(clk),
      .rst(rst | (hour_low_rst && ~tunemode) | (tunemode & (hour_low == 4'd9 || (hour_high == 2'd2 && hour_low == 4'd3)) & (arrow_edge[1] | arrow_edge[2]) & select == 3'd4)),
      .en   ((update & minute_high_rst& ~tunemode) | (tunemode & (arrow_edge[1] | arrow_edge[2]) & select == 3'd4)),
      .count(hour_low)
  );

  CounterAdd #(
      .WIDTH(2)
  ) u_counter_hour_high (
      .clk(clk),
      .rst(rst | hour_high_rst | (tunemode & (hour_high == 2'd2 || (hour_low >= 4'd4 && hour_high == 2'd1)) & (arrow_edge[1] | arrow_edge[2]) & select == 3'd5)),
      .en((update & hour_low_rst& ~tunemode) | (tunemode & (arrow_edge[1] | arrow_edge[2]) & select == 3'd5)),
      .count(hour_high)
  );

  wire [4:0] digrst;
  wire [3:0] stopwatch_count[0:4];
  genvar i;
  generate
    for (i = 0; i < 5; i = i + 1) begin
      if (i == 0) begin
        assign digrst[i] = update1000hz & stopwatch_count[i] == 4'd9;
        CounterAdd #(
            .WIDTH(4)
        ) i_stopwatch0 (
            .clk  (clk),
            .rst  (rst | ~stopwatch | digrst[i] | clear),
            .en   (update1000hz & stopwatch & ~pause),
            .count(stopwatch_count[i])
        );
      end else begin
        assign digrst[i] = stopwatch_count[i] == 4'd9 && digrst[i-1];
        CounterAdd #(
            .WIDTH(4)
        ) i_stopwatch1 (
            .clk  (clk),
            .rst  (rst | ~stopwatch | digrst[i] | clear),
            .en   (stopwatch & digrst[i-1] & ~pause),
            .count(stopwatch_count[i])
        );
      end

    end
  endgenerate

  bcd7seg o0 (
      .b(stopwatch ? stopwatch_count[0] : second_low),
      .h(dig0)
  );

  bcd7seg o1 (
      .b(stopwatch ? stopwatch_count[1] : {1'b0, second_high}),
      .h(dig1)
  );

  bcd7seg o2 (
      .b(stopwatch ? stopwatch_count[2] : 4'hf),
      .h(dig2)
  );
  bcd7seg o3 (
      .b(stopwatch ? 4'hf : minute_low),
      .h(dig3)
  );

  bcd7seg o4 (
      .b(stopwatch ? stopwatch_count[3] : {1'b0, minute_high}),
      .h(dig4)
  );

  bcd7seg o5 (
      .b(stopwatch ? stopwatch_count[4] : 4'hf),
      .h(dig5)
  );
  bcd7seg o6 (
      .b(stopwatch ? 4'hf : hour_low),
      .h(dig6)
  );

  bcd7seg o7 (
      .b(stopwatch ? 4'hf : {2'b0, hour_high}),
      .h(dig7)
  );


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
      default: h = 7'b1111111;
    endcase
  end

endmodule

module get_freq #(
    TICKS = 32'd500_000
) (
    clk,
    rst,
    pause,
    out
);
  input clk, rst, pause;
  output out;
  wire [31:0] count;
  wire        zero;
  CounterAdd #(
      .WIDTH(32)
  ) u_counter (
      .clk  (clk),
      .rst  (rst | zero),
      .en   (1'b1 & ~pause),
      .count(count)
  );
  // nvboard仿真跑不到 50MHz，改成500kHz
  assign zero = (count == TICKS - 32'd1);
  assign out  = zero;

endmodule

module CounterAdd #(
    WIDTH = 3
) (
    input clk,
    input rst,
    input en,
    output [WIDTH-1:0] count
);
  Reg #(
      .WIDTH(WIDTH)
  ) u_reg (
      .clk (clk),
      .rst (rst),
      .din (count + 'd1),
      .dout(count),
      .wen (en)
  );
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
