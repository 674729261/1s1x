#include <am.h>
#include <nemu.h>
#include <stdint.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  uint32_t ctl_lower = inl(VGACTL_ADDR);
  int w = ctl_lower >> 16;
  int h = ctl_lower & 0xFFFF;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i++)
    fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t ctl_lower = inl(VGACTL_ADDR);
  int w = ctl_lower >> 16;
  int h = ctl_lower & 0xFFFF;
  *cfg = (AM_GPU_CONFIG_T){.present = true,
                           .has_accel = false,
                           .width = w,
                           .height = h,
                           .vmemsz = w * h * sizeof(uint32_t)};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  for (int i = 0; i < ctl->w + ctl->h; i++) {
    outl(FB_ADDR, ((uint32_t *)ctl->pixels)[i]);
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) { status->ready = true; }
