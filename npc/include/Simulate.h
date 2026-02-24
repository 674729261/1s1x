#pragma once
#include "Ref.h"
#include <DUT.h>
#include <memory>
#include <print>
#include <replxx.h>
#include <verilated.h>
#include <verilated_vcd_c.h>
inline std::unique_ptr<Dut> dut;
inline std::unique_ptr<Ref> ref;
inline VerilatedContext contextp;

enum class SimulationState { RUNNING, QUIT, HALT, DIFFTEST_FAILED };
inline std::atomic<SimulationState> sim_state;

int simulate(int argc, char *argv[]);

void run(unsigned long long steps);