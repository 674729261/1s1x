#include "Simulators/NEMUemu.h"
#include "utils.h"
#include <cstdint>
#include <dlfcn.h>
#include <format>
#include <spdlog/spdlog.h>
#include <stdexcept>
NEMUemu::NEMUemu(size_t MemSize, std::string_view programe)
    : RISCV32(MemSize, programe, PC_Init) {
  loaded_lib = dlopen(STR(SO_PATH_NEMUemu), RTLD_LAZY);
  if (loaded_lib == nullptr) {
    spdlog::error("Failed to load library from {}", STR(SO_PATH_NEMUemu));
    throw std::runtime_error("Failed to load NEMUemu library");
  }
  difftest_memcpy = (difftest_memcpy_t)dlsym(loaded_lib, "difftest_memcpy");
  difftest_exec = (difftest_exec_t)dlsym(loaded_lib, "difftest_exec");
  difftest_init = (difftest_init_t)dlsym(loaded_lib, "difftest_init");
  difftest_regcpy = (difftest_regcpy_t)dlsym(loaded_lib, "difftest_regcpy");
  if (!difftest_memcpy || !difftest_exec || !difftest_init ||
      !difftest_regcpy) {
    spdlog::error("Failed to load exported symbols from {}",
                  STR(SO_PATH_NEMUemu));
    throw std::runtime_error("Failed to load NEMUemu symbols");
  }
}

void NEMUemu::reset() {
  difftest_init(1145);
  difftest_memcpy(PC_Init, M.data(), sizeof(uint32_t) * M.size(), 1);
}

void NEMUemu::step(bool display) {
  difftest_exec(1);
  inst_count++;
  need_sync = true;
}
void NEMUemu::syncCPUState() { difftest_regcpy(&cpu, 0); }
uint32_t NEMUemu::getGPR(int idx) {
  if (need_sync) {
    syncCPUState();
    need_sync = false;
  }
  if (idx >= 0 && idx < 32)
    return cpu.gpr[idx];
  else if (idx == 32)
    return cpu.pc;
  else
    throw std::logic_error(std::format("Invalid idx gpr {}", idx));
}
uint32_t NEMUemu::getPC() {
  if (need_sync) {
    syncCPUState();
    need_sync = false;
  }
  return cpu.pc;
}
NEMUemu::~NEMUemu() {
  if (loaded_lib != nullptr)
    dlclose(loaded_lib);
}
uint32_t NEMUemu::readMemory(int raddr) {
  throw std::logic_error("Can not read NEMUemu memory");
}
void NEMUemu::writeMemory(int waddr, int wdata, char wmask) {
  throw std::logic_error("Can not write NEMUemu memory");
}

int NEMUemu::instrCount() { return inst_count; }