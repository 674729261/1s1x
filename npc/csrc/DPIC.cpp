#include "Setup.h"
#include "my_utils.h"
#include "spdlog/spdlog.h"
#include <Mem.h>
#include <cstdint>
#include <stdexcept>

extern "C" uint32_t mem_read(uint32_t raddr) {
  if (raddr >= config.base_memory &&
      raddr < config.base_memory + config.mem_size) {
    // in memory space
    size_t index = (raddr - config.base_memory) >> 2;
    spdlog::info("addr : {:08x}, data : {:08x}", raddr, mem[index]);
    return mem[index];
  } else if (raddr >= config.base_device &&
             raddr < config.base_device + config.device_size) {
    // in MMIO
    uint32_t offset = raddr - config.base_device;
    if (offset == SERIAL_OFFSET) {
      log_and_throw<std::logic_error>("Serial cannot be read");
    } else {
      todo("MMIO");
    }
  } else {
    return 0xdeadbeef;
  }
}

extern "C" void mem_write(uint32_t waddr, uint32_t wmask, uint32_t wdata) {
  if (waddr >= config.base_memory &&
      waddr < config.base_memory + config.mem_size) {
    // in memory space
    size_t index = (waddr - config.base_memory) >> 2;
    uint32_t mask32 = lookup_mask32[wmask];
    mem[index] &= ~mask32;
    mem[index] |= wdata & mask32;
  } else if (waddr >= config.base_device &&
             waddr < config.base_device + config.device_size) {
    // in MMIO
    uint32_t offset = waddr - config.base_device;
    if (offset == SERIAL_OFFSET) {
      std::cout.put(wdata);
    } else {
      todo("MMIO");
    }

  } else {
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}] "
        "[{:#010x},{:#010x}]",
        waddr, config.base_memory, config.base_memory + config.mem_size - 1,
        config.base_device, config.base_device + config.device_size - 1);
  }
}