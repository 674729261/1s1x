#include "inst.h"
#include <am.h>
#include <klib-macros.h>
#include <stdint.h>
#include <stdio.h>

extern void load_inst(const char *filename);
extern void inst_cycle();
uint32_t canvas[N * N];
void draw() {
  static uint32_t color_buf[64 * 64];
  int w = io_read(AM_GPU_CONFIG).width / N;
  int h = io_read(AM_GPU_CONFIG).height / N;
  for (int x = 0; x < N; x++) {
    for (int y = 0; y < N; y++) {
      for (int k = 0; k < w * h; ++k) {
        color_buf[k] = canvas[y * N + x];
      }
      io_write(AM_GPU_FBDRAW, x * w, y * h, color_buf, w, h, false);
    }
  }
}

int main(void) {
  ioe_init(); // initialization for GUI
  unsigned long long last = 0;
  unsigned long long fps = 30;

  load_inst(
      "/home/shitful/1s1x/ysyx-workbench/c_practice/prepro/vga/hex/vga.data");
  printf("Program file loaded successfully.\n");
  while (1) {

    unsigned long long upt = io_read(AM_TIMER_UPTIME).us;
    if (upt - last > 1000000 / fps) {
      last = upt;
      draw();
    }

    inst_cycle();
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if (ev.keycode == AM_KEY_ESCAPE) {
      break;
    }
  }
  return 0;
}