
module top_e2 (
    enc_input,
    dec_input,
    enc_output,
    dec_output,
    penc_input,
    penc_output,
    led,
    en
);

  input [3:0] enc_input;
  input [7:0] penc_input;
  input [1:0] dec_input;
  output [1:0] enc_output;
  output [2:0] penc_output;
  output [3:0] dec_output;
  output [6:0] led;
  input en;

  decode24 u_decode24 (
      .x (dec_input),
      .en(en),
      .y (dec_output)
  );


  encode42 u_encode42 (
      .x (enc_input),
      .en(en),
      .y (enc_output)
  );

  prior_encoder_83 p_encode42 (
      .x (penc_input),
      .en(en),
      .y (penc_output)
  );
  bcd7seg u_bcd7seg (
      .b({1'b0, penc_output}),
      .h(led)
  );


endmodule

module decode24 (
    x,
    en,
    y
);
  input [1:0] x;
  input en;
  output reg [3:0] y;

  always @(x or en)
    if (en) begin
      case (x)
        2'd0: y = 4'b0001;
        2'd1: y = 4'b0010;
        2'd2: y = 4'b0100;
        2'd3: y = 4'b1000;
      endcase
    end else y = 4'b0000;

endmodule

module encode42 (
    x,
    en,
    y
);
  input [3:0] x;
  input en;
  output reg [1:0] y;

  always @(x or en) begin
    if (en) begin
      case (x)
        4'b0001: y = 2'b00;
        4'b0010: y = 2'b01;
        4'b0100: y = 2'b10;
        4'b1000: y = 2'b11;
        default: y = 2'b00;
      endcase
    end else y = 2'b00;
  end
endmodule

module prior_encoder_83 (
    x,
    en,
    y
);
  input [7:0] x;
  input en;
  output reg [2:0] y;

  always @(x or en) begin
    if (en) begin
      casez (x)
        8'b00000001: y = 3'b000;
        8'b0000001?: y = 3'b001;
        8'b000001??: y = 3'b010;
        8'b00001???: y = 3'b011;
        8'b0001????: y = 3'b100;
        8'b001?????: y = 3'b101;
        8'b01??????: y = 3'b110;
        8'b1???????: y = 3'b111;
        default: y = 3'b000;
      endcase
    end else y = 3'b00;
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
