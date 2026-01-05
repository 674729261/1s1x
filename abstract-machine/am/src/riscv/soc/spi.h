#ifndef SPI_H_
#define SPI_H_
#include "soc.h"
#include <riscv/riscv.h>
#define SPI_SLAVE_FLASH (1 << 0)

static inline void spi_init() {
  uint32_t ctrl = inw(SPI_BASE + SPI_CTRL);
  ctrl = 0x0240;
  outw(SPI_BASE + SPI_DIV, 0x0000000f);
  outw(SPI_BASE + SPI_CTRL, ctrl);
}
static inline void wait_spi_finish() {
  uint32_t ctrl = *(volatile uint32_t *)(SPI_BASE + SPI_CTRL);
  while (ctrl & (1 << 8))
    ctrl = *(volatile uint32_t *)(SPI_BASE + SPI_CTRL);
}
static inline void set_spi_ss(uint32_t slave) {
  outw(SPI_BASE + SPI_SS, slave);
}
static inline void set_spi_div(uint32_t divider) {
  outw(SPI_BASE + SPI_DIV, divider);
}
static inline void set_spi_busy() {
  uint32_t ctrl = inw(SPI_BASE + SPI_CTRL);
  ctrl |= (1 << 8);
  outw(SPI_BASE + SPI_CTRL, ctrl);
}

static inline void set_spi_len(uint32_t len) {
  assert(len >= 1 && len <= 128);
  if (len == 128)
    len = 0;
  uint32_t ctrl = inw(SPI_BASE + SPI_CTRL);
  ctrl &= ~0x7f;
  ctrl |= len;
  outw(SPI_BASE + SPI_CTRL, ctrl);
}
#endif