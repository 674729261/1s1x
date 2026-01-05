#include <Flash.h>
#include <cstdint>
#include <print>

extern "C" void flash_read(int32_t addr, int32_t *data) {
  std::println("!!");
  *data = (addr & 0x0FFFFFFF);
}

void init_flash() {}