#pragma once

#include "my_utils.h"
#include <Device/Device.h>
#include <cstdint>
#include <stdexcept>
#include <vector>
class Memory {
public:
  Memory(int size) : mem(size) {}

  uint32_t readMemory(uint32_t raddr, Devices *devices) {

    // #ifndef DISABLE_ALL_TRACER
    //     if (mtracer) {
    //       println("Reading memory memory : {:#010x}", (uint32_t)raddr);
    //     }
    // #endif
    uint32_t rdata = 0xdeadbeef;

    uint32_t addr = (uint32_t)(raddr - Devices::PC_Init) >> 2;
    if (addr < mem.size() && raddr >= Devices::PC_Init) [[likely]]
      rdata = mem[addr];
    else if (raddr >= Devices::deviceBase) {
      if (devices)
        devices->readMMIO(raddr);
      else
        log_and_throw<std::logic_error>(
            "Accessing devices while devices are disabled");
    }

    return rdata;
  }

  void writeMemory(uint32_t waddr, uint32_t wdata, uint32_t wmask,
                   Devices *devices) {
    constexpr std::array<uint32_t, 16> lookup_mask32 = {
        0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF, 0x00FF0000, 0x00FF00FF,
        0x00FFFF00, 0x00FFFFFF, 0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
        0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF};
    uint32_t mask32 = lookup_mask32[wmask]; // extend 4bit mask to 32bit mask

    // #ifndef DISABLE_ALL_TRACER
    //   if (mtracer) {
    //     println("Write to memory : {:#010x}, data : {:#010x}, mask :
    //     {:#010x}",
    //             (uint32_t)waddr, (uint32_t)wdata, (uint32_t)wmask);
    //   }
    // #endif

    uint32_t addr = (uint32_t)(waddr - Devices::memOffset) >> 2;
    if (addr < mem.size() && waddr >= Devices::memOffset) [[likely]] {
      mem[addr] &= ~mask32;
      mem[addr] |= wdata & mask32;
    } else if (waddr >= Devices::deviceBase) {
      if (devices)
        devices->writeMMIO(waddr, mask32, wdata);
      else
        log_and_throw<std::logic_error>(
            "Accessing devices while devices are disabled");
    }
  }

private:
  std::vector<uint32_t> mem;
};