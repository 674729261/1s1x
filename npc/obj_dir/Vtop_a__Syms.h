// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VTOP_A__SYMS_H_
#define VERILATED_VTOP_A__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vtop_a.h"

// INCLUDE MODULE CLASSES
#include "Vtop_a___024root.h"

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES)Vtop_a__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vtop_a* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vtop_a___024root               TOP;

    // CONSTRUCTORS
    Vtop_a__Syms(VerilatedContext* contextp, const char* namep, Vtop_a* modelp);
    ~Vtop_a__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
