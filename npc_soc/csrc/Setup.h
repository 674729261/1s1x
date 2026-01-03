#pragma once
#include "Device/Device.h"
#include "Simulators/RISCV32.h"
#include <Simulators/NEMUemu.h>
#include <Simulators/NPCemu.h>
#include <Vnpc_top.h>
#include <argparse/argparse.hpp>
#include <cstdint>
#include <memory>
#include <print>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

extern std::shared_ptr<RISCV32> emu;
// extern std::shared_ptr<NEMUemu> nemu;
extern std::shared_ptr<RISCV32> refemu;

void register_argparse(argparse::ArgumentParser &program);
void register_logger(argparse::ArgumentParser &program);
struct Config {
  std::string image_path;
  bool batch_mode;
  bool difftest;
};

Config setup(argparse::ArgumentParser &program);