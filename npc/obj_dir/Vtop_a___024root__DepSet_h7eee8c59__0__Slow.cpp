// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop_a.h for the primary calling header

#include "Vtop_a__pch.h"
#include "Vtop_a__Syms.h"
#include "Vtop_a___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop_a___024root___dump_triggers__stl(Vtop_a___024root* vlSelf);
#endif  // VL_DEBUG

VL_ATTR_COLD void Vtop_a___024root___eval_triggers__stl(Vtop_a___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop_a___024root___eval_triggers__stl\n"); );
    Vtop_a__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__VstlTriggered.setBit(0U, (IData)(vlSelfRef.__VstlFirstIteration));
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vtop_a___024root___dump_triggers__stl(vlSelf);
    }
#endif
}
