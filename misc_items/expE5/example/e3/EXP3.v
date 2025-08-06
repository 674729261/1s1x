module ALU #(
    BITWIDTH = 4
) (
    A,
    B,
    Sel,
    Out,
    Cout,
    Zero,
    Overflow
);
  input [BITWIDTH-1:0] A, B;
  output reg [BITWIDTH-1:0] Out;
  input [2:0] Sel;
  output Cout, Zero, Overflow;

  localparam ADD = 3'b000;
  localparam SUB = 3'b001;
  localparam NOT = 3'b010;
  localparam AND = 3'b011;
  localparam OR = 3'b100;
  localparam XOR = 3'b101;
  localparam LESS = 3'b110;
  localparam EQU = 3'b111;

  wire [BITWIDTH-1:0] Out_temp;
  ;

  addsuber #(
      .BITWIDTH(4)
  ) addsuber_inst (
      .A       (A),
      .B       (B),
      .Sel     (Sel == SUB || Sel == EQU || Sel == LESS),
      .Out     (Out_temp),
      .Cout    (Cout),
      .Zero    (Zero),
      .Overflow(Overflow)
  );

  always @(*) begin
    case (Sel)
      ADD: Out = Out_temp;
      SUB: Out = Out_temp;
      NOT: Out = ~A;
      AND: Out = A & B;
      OR: Out = A | B;
      XOR: Out = A ^ B;
      EQU: Out = Out_temp == 0 ? 4'b0001 : 4'b0000;
      LESS: Out = Out_temp[BITWIDTH-1] ^ Overflow ? 4'b0001 : 4'b0000;
      default: Out = 4'b0000;
    endcase
  end

endmodule


module addsuber #(
    BITWIDTH = 32
) (
    A,
    B,
    Sel,  // 0 : add, 1 : sub
    Out,
    Cout,
    Zero,
    Overflow
);
  input [BITWIDTH-1:0] A, B;
  output [BITWIDTH-1:0] Out;
  input Sel;
  output Cout, Zero, Overflow;

  wire [BITWIDTH-1:0] B_inv;
  assign B_inv = ~B;

  bit_adder #(
      .BITWIDTH(BITWIDTH)
  ) adder (
      .A   (A),
      .B   (Sel ? B_inv : B),
      .Cin (Sel),
      .S   (Out),
      .Cout(Cout)
  );

  assign Zero = (Out == 0);
  wire add_overflow;
  assign add_overflow = (A[BITWIDTH-1] == B[BITWIDTH-1]) && (Out[BITWIDTH-1] != A[BITWIDTH-1]);
  wire sub_overflow;
  assign sub_overflow = (A[BITWIDTH-1] != B[BITWIDTH-1]) && (Out[BITWIDTH-1] != A[BITWIDTH-1]);

  assign Overflow = Sel ? sub_overflow : add_overflow;
endmodule

module bit_adder #(
    BITWIDTH = 32
) (
    A,
    B,
    Cin,
    S,
    Cout
);
  input [BITWIDTH-1:0] A, B;
  input Cin;
  output [BITWIDTH-1:0] S;
  wire [BITWIDTH-1:0] carry;
  output Cout;

  assign carry[0] = Cin;

  genvar i;
  generate
    for (i = 0; i < BITWIDTH; i = i + 1) begin : gen_full_adder
      if (i < BITWIDTH - 1) begin
        full_adder u_full_adder (
            .A   (A[i]),
            .B   (B[i]),
            .Cin (carry[i]),
            .S   (S[i]),
            .Cout(carry[i+1])
        );
      end else begin
        full_adder u_full_adder (
            .A   (A[i]),
            .B   (B[i]),
            .Cin (carry[i]),
            .S   (S[i]),
            .Cout(Cout)
        );
      end
    end
  endgenerate


endmodule

module full_adder (
    A,
    B,
    Cin,
    S,
    Cout
);
  input A, B, Cin;
  output S, Cout;

  assign S = A ^ B ^ Cin;
  assign Cout = (A & B) | (B & Cin) | (A & Cin);

endmodule
