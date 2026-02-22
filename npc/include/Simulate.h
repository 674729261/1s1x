#pragma once
#include <DUT.h>
#include <memory>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>
inline std::unique_ptr<Dut> dut;
inline VerilatedContext contextp;

enum class SimulationState { RUNNING, QUIT, HALT, DIFFTEST_FAILED };
inline std::atomic<SimulationState> sim_state;