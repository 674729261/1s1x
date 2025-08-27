#ifndef __PORT_H__
#define __PORT_H__

#include <cstdint>
#include <optional>
#define DEVICE_BASE 0xa0000000
#define MMIO_BASE 0xa0000000

#define SERIAL_PORT (DEVICE_BASE + 0x00003f8)
#define KBD_ADDR (DEVICE_BASE + 0x0000060)
#define RTC_ADDR (DEVICE_BASE + 0x0000048)
#define VGACTL_ADDR (DEVICE_BASE + 0x0000100)
#define AUDIO_ADDR (DEVICE_BASE + 0x0000200)
#define DISK_ADDR (DEVICE_BASE + 0x0000300)
#define FB_ADDR (MMIO_BASE + 0x1000000)
#define AUDIO_SBUF_ADDR (MMIO_BASE + 0x1200000)

#define RTC_ADDR_END (DEVICE_BASE + 0x0000048 + 0x8)

extern uint32_t RTC_reg[2];

int write_mmio(uint32_t waddr, uint32_t mask32, uint32_t wdata);
std::optional<uint32_t> read_mmio(uint32_t raddr);

#endif