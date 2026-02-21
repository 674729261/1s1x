#include <Mem.h>

extern "C" uint32_t mem_read(uint32_t raddr) {
  size_t index = (raddr - config.base_memory) >> 2;
  if (index >= mem.size() || raddr < config.base_memory)
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}]", raddr,
        config.base_memory,
        config.base_memory + config.mem_size * sizeof(uint32_t) - 1);
  return mem[index];
}

extern "C" void mem_write(uint32_t waddr, uint32_t wmask, uint32_t wdata) {
  size_t index = (waddr - config.base_memory) >> 2;
  if (index >= mem.size() || waddr < config.base_memory)
    log_and_throw<std::logic_error>(
        "Mem index {:#010x} out of range [{:#010x},{:#010x}]", waddr,
        config.base_memory,
        config.base_memory + config.mem_size * sizeof(uint32_t) - 1);
  uint32_t mask32 = lookup_mask32[wmask];
  mem[index] &= ~mask32;
  mem[index] |= wdata & mask32;
}