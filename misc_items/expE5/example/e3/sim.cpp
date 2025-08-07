#include <VALU.h>
#include <cstdint>
#include <cstdio>
#include <nvboard.h>
#include <random>
static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME *top);

int32_t sign_extend(int value, int bits) {
  return (value << (32 - bits)) >> (32 - bits);
}

int main() {
  // 自动测试
  std::mt19937 rng(0x12345678);
  int error_count = 0;
  const int iters = 1024;
  int total_cases = 0;
  for (int i = 0; i < iters; i++) {
    for (int sel = 0; sel < 8; ++sel) {
      total_cases++;
      dut.Sel = sel;
      switch (sel) {
      case 0: // ADD
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        int ans = A + B;
        dut.eval();
        if (((ans & 0x10) >> 4) != dut.Cout) {
          printf("Error: A = %d, B = %d, Sel=%d, Cout = %d, expected = %d\n", A,
                 B, sel, dut.Cout, ((ans & 0x10) >> 4));
          error_count++;
        }
        A = sign_extend(A, 4);
        B = sign_extend(B, 4);
        ans = A + B;

        if (dut.Out != (ans & 0xF)) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, ans & 0xF);
          error_count++;
        }

        if ((ans > 7 || ans < -8) != dut.Overflow) {
          printf(
              "Error: A = %d, B = %d, Sel=%d, Overflow = %d, expected = %d\n",
              A, B, sel, dut.Overflow, (ans > 15 || ans < -16));
          error_count++;
        }
      } break;
      case 1: // SUB
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        int ans = A + (B ^ 0xF) + 1;
        dut.eval();
        if (((ans & 0x10) >> 4) != dut.Cout) {
          printf("Error: A = %d, B = %d, Sel=%d, Cout = %d, expected = %d\n", A,
                 B, sel, dut.Cout, ((ans & 0x10) >> 4));
          error_count++;
        }
        A = sign_extend(A, 4);
        B = sign_extend(B, 4);
        ans = A - B;

        if (dut.Out != (ans & 0xF)) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, ans & 0xF);
          error_count++;
        }

        if ((ans > 7 || ans < -8) != dut.Overflow) {
          printf(
              "Error: A = %d, B = %d, Sel=%d, Overflow = %d, expected = %d\n",
              A, B, sel, dut.Overflow, (ans > 15 || ans < -16));
          error_count++;
        }
      } break;
      case 2: // NOT
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        int ans = (~A) & 0xF;
        dut.eval();
        if (ans != dut.Out) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, ans & 0xF);
          error_count++;
        }
      } break;
      case 3: // AND
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        int ans = A & B;
        dut.eval();
        if ((ans & 0xF) != dut.Out) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, ans & 0xF);
          error_count++;
        }
      } break;
      case 4: // OR
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        int ans = A | B;
        dut.eval();
        if ((ans & 0xF) != dut.Out) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, ans & 0xF);
          error_count++;
        }
      } break;
      case 5: // XOR
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        int ans = A ^ B;
        dut.eval();
        if ((ans & 0xF) != dut.Out) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, ans & 0xF);
          error_count++;
        }
      } break;
      case 6: // LESS
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        dut.eval();
        A = sign_extend(A, 4);
        B = sign_extend(B, 4);
        if ((A < B) != dut.Out) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, A < B);
          error_count++;
        }
      } break;
      case 7: // EQU
      {
        int A = rng() & 0xF;
        int B = rng() & 0xF;

        dut.A = A;
        dut.B = B;
        dut.eval();
        if ((A == B) != dut.Out) {
          printf("Error: A = %d, B = %d, Sel=%d, Out = %d, expected = %d\n", A,
                 B, sel, dut.Out, A < B);
          error_count++;
        }
      } break;
      }
    }
  }
  printf("Test completed with %d errors out of %d cases.\n", error_count,
         total_cases);
  // 上板测试
  nvboard_bind_all_pins(&dut);
  nvboard_init();
  uint8_t all_high = 0xff;
  nvboard_bind_pin(&all_high, 7, SEG3A, SEG3B, SEG3C, SEG3D, SEG3E, SEG3F,
                   SEG3G);
  while (1) {
    nvboard_update();
    dut.eval();
  }
}