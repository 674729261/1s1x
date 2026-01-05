#ifndef SPI_H_
#define SPI_H_
#include <riscv/riscv.h>
#include <soc.h>
#include <stdint.h>
#define SPI_SLAVE_FLASH (1 << 0)

static inline void spi_init() {
  uint32_t ctrl = inw(SPI_BASE + SPI_CTRL);
  ctrl = 0x0640;
  outw(SPI_BASE + SPI_DIV, 0x0000000f);
  outw(SPI_BASE + SPI_CTRL, ctrl);
}

static inline void spi_tx_neg(bool neg) {
  uint32_t ctrl = inw(SPI_BASE + SPI_CTRL);
  ctrl &= ~(1 << 10);
  if (neg)
    ctrl |= (1 << 10);
  outw(SPI_BASE + SPI_CTRL, ctrl);
}

static inline void spi_rx_neg(bool neg) {
  uint32_t ctrl = inw(SPI_BASE + SPI_CTRL);
  ctrl &= ~(1 << 9);
  if (neg)
    ctrl |= (1 << 9);
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

static inline uint32_t flash_read(uintptr_t addr) {
  spi_tx_neg(true);
  spi_rx_neg(true);
  outw(SPI_BASE + 0x4, (0x03 << 24) | (addr & 0x00ffffff));
  set_spi_ss(SPI_SLAVE_FLASH);
  set_spi_len(64);
  set_spi_busy();
  wait_spi_finish();
  uint32_t rx_data = inw(SPI_BASE);
  return rx_data;
}

#endif