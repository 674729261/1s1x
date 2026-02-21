#include "Setup.h"
#include "my_utils.h"
#include <Mem.h>

extern "C" uint32_t mem_read(uint32_t raddr) {
  if (raddr >= config.base_memory &&
      raddr < config.base_memory + config.mem_size) {
    // in memory space
    size_t index = (raddr - config.base_memory) >> 2;
    return mem[index];
  } else if (raddr >= config.base_device &&
             raddr < config.base_device + config.device_size) {
    // in MMIO
    todo("MMIO");
  } else {
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}] "
        "[{:#010x},{:#010x}]",
        raddr, config.base_memory, config.base_memory + config.mem_size - 1,
        config.base_device, config.base_device + config.device_size - 1);
  }
}

extern "C" void mem_write(uint32_t waddr, uint32_t wmask, uint32_t wdata) {
  size_t index = (waddr - config.base_memory) >> 2;
  if (index >= mem.size() || waddr < config.base_memory)
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}]", waddr,
        config.base_memory, config.base_memory + config.mem_size - 1);
  uint32_t mask32 = lookup_mask32[wmask];
  mem[index] &= ~mask32;
  mem[index] |= wdata & mask32;
}