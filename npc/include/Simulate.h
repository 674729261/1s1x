#pragma once
#include "Ref.h"
#include <DUT.h>
#include <memory>
#include <print>
#include <replxx.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

extern std::unique_ptr<Dut> dut;
extern std::unique_ptr<Ref> ref;
extern std::unique_ptr<VerilatedContext> contextp;

enum class SimulationState { RUNNING, HALT, DIFFTEST_FAILED };
extern SimulationState sim_state;

int simulate(int argc, char *argv[]);

void run(unsigned long long steps);