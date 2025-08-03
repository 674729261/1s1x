// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop_a.h for the primary calling header

#include "Vtop_a__pch.h"
#include "Vtop_a__Syms.h"
#include "Vtop_a___024root.h"

void Vtop_a___024root___ctor_var_reset(Vtop_a___024root* vlSelf);

Vtop_a___024root::Vtop_a___024root(Vtop_a__Syms* symsp, const char* v__name)
    : VerilatedModule{v__name}
    , vlSymsp{symsp}
 {
    // Reset structure values
    Vtop_a___024root___ctor_var_reset(this);
}

void Vtop_a___024root::__Vconfigure(bool first) {
    (void)first;  // Prevent unused variable warning
}

Vtop_a___024root::~Vtop_a___024root() {
}
