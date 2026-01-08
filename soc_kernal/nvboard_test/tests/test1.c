#include "am.h"
#include "amdev.h"
#include <klib.h>
#include <stdint.h>
#include <stdio.h>

#define LED_GPIO 0x10002000
#define SWITCH_GPIO 0x10002004
#define DIGIT_GPIO 0x10002008

const uint8_t hex_2_digit_ctrl[16] = {0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49,
                                      0x41, 0x1F, 0x01, 0x09, 0x11, 0xC1,
                                      0x63, 0x85, 0x61, 0x71};

int main() {
  uint32_t marchid;
  asm volatile("csrr %0, marchid" : "=r"(marchid));
  for (int i = 0; i < 8; i++) {
    *(volatile uint8_t *)(DIGIT_GPIO + i) = hex_2_digit_ctrl[marchid % 10];
    marchid /= 10;
  }
  AM_UART_RX_T rx;
  ioe_read(AM_UART_RX, &rx);
  uint32_t x = rx.data - '0';
  ioe_read(AM_UART_RX, &rx);
  x = x * 10 + rx.data - '0';
  ioe_read(AM_UART_RX, &rx);
  x = x * 10 + rx.data - '0';
  printf("%u + \n", x);

  ioe_read(AM_UART_RX, &rx);
  uint32_t y = rx.data - '0';
  ioe_read(AM_UART_RX, &rx);
  y = y * 10 + rx.data - '0';
  ioe_read(AM_UART_RX, &rx);
  y = y * 10 + rx.data - '0';
  printf("%u = %u\n", y, x + y);

loop:
  for (uint16_t p = 0; p < 16; p++) {
    *(volatile uint16_t *)LED_GPIO = (1 << p);
    while (*(volatile uint16_t *)SWITCH_GPIO != 0x8001)
      ;
    putch(p < 10 ? '0' + p : 'A' + p - 10);
    putch('\n');
    for (volatile int j = 0; j < 1024; j++)
      ;
  }
  goto loop;
  return 0;
}
