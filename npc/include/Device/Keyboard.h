#pragma once
#include "lockfree/spsc/queue.hpp"
#include <cstdint>
#include <memory>
struct KeyboardBase_t {
  using KBD_BUF = lockfree::spsc::Queue<uint32_t, 128>;
  std::unique_ptr<KBD_BUF> kbd_buf;
};

inline KeyboardBase_t Keyboard;