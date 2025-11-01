#include <Simulators/NEMUemu.h>
#include <cstdint>
#include <dlfcn.h>
#include <format>
#include <my_utils.h>
#include <print>
#include <spdlog/spdlog.h>
#include <stdexcept>
NEMUemu::NEMUemu() : RISCV32() {
  loaded_lib = dlopen(STR(SO_PATH_NEMU), RTLD_LAZY);
  if (loaded_lib == nullptr) {
    log_and_throw<std::runtime_error>("Failed to load library from {}",
                                      STR(SO_PATH_NEMU));
  }
  difftest_memcpy = (difftest_memcpy_t)dlsym(loaded_lib, "difftest_memcpy");
  difftest_exec = (difftest_exec_t)dlsym(loaded_lib, "difftest_exec");
  difftest_init = (difftest_init_t)dlsym(loaded_lib, "difftest_init");
  difftest_regcpy = (difftest_regcpy_t)dlsym(loaded_lib, "difftest_regcpy");
  if (!difftest_memcpy || !difftest_exec || !difftest_init ||
      !difftest_regcpy) {
    log_and_throw<std::runtime_error>("Failed to load exported symbols from {}",
                                      STR(SO_PATH_NEMUemu));
  }
}

void NEMUemu::reset() {
  difftest_init(1145);
  difftest_memcpy(PC_Init, devices->M.data(),
                  sizeof(uint32_t) * devices->M.size(), 1);
}

void NEMUemu::step() {
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
    log_and_throw<std::logic_error>("Invalid idx gpr {}", idx);
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

unsigned long long NEMUemu::instrCount() { return inst_count; }