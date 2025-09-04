#include "Capstone.h"
#include "utils.h"
#include <cstddef>
#include <dlfcn.h>
#include <format>
#include <print>
#include <spdlog/spdlog.h>
#include <stdexcept>

bool Capstone::load_libcapstone() {
  auto lib = dlopen(STR(SO_PATH_CAPSTONE), RTLD_LAZY);
  if (lib == nullptr) {
    spdlog::error("Failed to load from {}", STR(SO_PATH_CAPSTONE));
    return false;
  }
  cserr_fn_type cs_open_dl = NULL;
  cs_open_dl = (cserr_fn_type)dlsym(lib, "cs_open");
  if (cs_open_dl == nullptr) {
    dlclose(lib);
    return false;
  }

  cs_disasm_dl = (disasm_fn_type)dlsym(lib, "cs_disasm");
  if (cs_disasm_dl == nullptr) {
    dlclose(lib);
    cs_open_dl = nullptr;
    return false;
  }
  int ret = cs_open_dl(CS_ARCH_RISCV, CS_MODE_RISCV32, &handle);
  if (ret == 0) {
    dlclose(lib);
    cs_open_dl = nullptr;
    cs_disasm_dl = nullptr;
    return false;
  }
  return true;
}

std::string Capstone::disassemble(int size, uint64_t pc, uint8_t *code,
                                  int nbyte, bool display) {
  if (cs_disasm_dl == nullptr)
    throw std::runtime_error("Did not load libcapstone first");
  cs_insn *insn;
  size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
  if (count != 1)
    throw std::logic_error(std::format("Invalid instruction@{:#010x}", pc));
  std::string str;
  if (insn->op_str[0] != '\0') {
    str = std::format("{}\t{}", insn->mnemonic, insn->op_str);
  }
  if (display)
    println("{}", str);
  cs_free_dl(insn, count);
  return str;
}