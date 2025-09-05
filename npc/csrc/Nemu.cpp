#include "Simulators/Nemu.h"
#include "Simulators/RISCV32.h"
#include "utils.h"
#include <dlfcn.h>
Nemu::Nemu(size_t MemSize, std::string_view programe)
    : RISCV32(MemSize, programe, PC_Init) {
  loaded_lib = dlopen(STR(SO_PATH_NEMU), RTLD_LAZY);
}