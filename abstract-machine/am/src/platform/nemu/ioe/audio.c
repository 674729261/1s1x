#include "riscv/riscv.h"
#include <am.h>
#include <klib.h>
#include <nemu.h>
#include <stdint.h>

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)

static int audio_sbuf_flag = 0;

void __am_audio_init() {}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  cfg->present = true;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, true);
  audio_sbuf_flag = 0;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int data_len = ctl->buf.end - ctl->buf.start;
  int sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  assert(data_len < sbuf_size);

  while (data_len > sbuf_size - inl(AUDIO_COUNT_ADDR)) {
  };
  uint8_t *buf = (uint8_t *)AUDIO_SBUF_ADDR;
  if (data_len + audio_sbuf_flag < sbuf_size) {
    memcpy(buf + audio_sbuf_flag, ctl->buf.start, data_len);
    audio_sbuf_flag += data_len;
  } else {
    memcpy(buf + audio_sbuf_flag, ctl->buf.start, sbuf_size - audio_sbuf_flag);
    memcpy(buf, ctl->buf.start + (sbuf_size - audio_sbuf_flag),
           data_len - (sbuf_size - audio_sbuf_flag));
    audio_sbuf_flag = data_len - (sbuf_size - audio_sbuf_flag);
  }
  int count = inl(AUDIO_COUNT_ADDR);
  count += data_len;
  outl(AUDIO_COUNT_ADDR, count);
}

// void __am_audio_init() {}

// void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
//   cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
//   cfg->present = cfg->bufsize > 0;
// }

// void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
//   outl(AUDIO_FREQ_ADDR, ctrl->freq);
//   outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
//   outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
//   outl(AUDIO_INIT_ADDR, 1);
// }

// void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
//   stat->count = inl(AUDIO_COUNT_ADDR);
// }

// void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
//   uint8_t *cur_addr = ctl->buf.start;
//   uint32_t len = ctl->buf.end - ctl->buf.start;
//   printf("%d %d %d\n", inl(AUDIO_SBUF_SIZE_ADDR), inl(AUDIO_COUNT_ADDR),
//   len); while (inl(AUDIO_SBUF_SIZE_ADDR) - inl(AUDIO_COUNT_ADDR) < len)
//     ;
//   uint32_t offset = inl(AUDIO_COUNT_ADDR);
//   while ((uint8_t *)ctl->buf.end - cur_addr >= 4) {
//     outl(AUDIO_SBUF_ADDR + offset, *(uint32_t *)cur_addr);
//     cur_addr += 4;
//     offset += 4;
//   }
//   while ((uint8_t *)ctl->buf.end - cur_addr >= 1) {
//     outb(AUDIO_SBUF_ADDR + offset, *(uint8_t *)cur_addr);
//     cur_addr++;
//     offset++;
//   }
//   outl(AUDIO_COUNT_ADDR, offset);
// }
