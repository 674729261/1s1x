// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vtop_a__pch.h"

//============================================================
// Constructors

Vtop_a::Vtop_a(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vtop_a__Syms(contextp(), _vcname__, this)}
    , a{vlSymsp->TOP.a}
    , b{vlSymsp->TOP.b}
    , f{vlSymsp->TOP.f}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vtop_a::Vtop_a(const char* _vcname__)
    : Vtop_a(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vtop_a::~Vtop_a() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vtop_a___024root___eval_debug_assertions(Vtop_a___024root* vlSelf);
#endif  // VL_DEBUG
void Vtop_a___024root___eval_static(Vtop_a___024root* vlSelf);
void Vtop_a___024root___eval_initial(Vtop_a___024root* vlSelf);
void Vtop_a___024root___eval_settle(Vtop_a___024root* vlSelf);
void Vtop_a___024root___eval(Vtop_a___024root* vlSelf);

void Vtop_a::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vtop_a::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vtop_a___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vtop_a___024root___eval_static(&(vlSymsp->TOP));
        Vtop_a___024root___eval_initial(&(vlSymsp->TOP));
        Vtop_a___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vtop_a___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vtop_a::eventsPending() { return false; }

uint64_t Vtop_a::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* Vtop_a::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vtop_a___024root___eval_final(Vtop_a___024root* vlSelf);

VL_ATTR_COLD void Vtop_a::final() {
    Vtop_a___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vtop_a::hierName() const { return vlSymsp->name(); }
const char* Vtop_a::modelName() const { return "Vtop_a"; }
unsigned Vtop_a::threads() const { return 1; }
void Vtop_a::prepareClone() const { contextp()->prepareClone(); }
void Vtop_a::atClone() const {
    contextp()->threadPoolpOnClone();
}
