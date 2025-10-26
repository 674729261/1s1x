#pragma once
#include "Capstone.h"
#include "Monitor/SingleMonitor.h"
#include "Simulators/NEMUemu.h"
#include "Simulators/NPCemu.h"
#include "spdlog/common.h"
#include <VCPU.h>
#include <argparse/argparse.hpp>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <ostream>
#include <print>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

extern std::shared_ptr<NPCemu> emu;
extern std::shared_ptr<NEMUemu> nemu;
void register_argparse(argparse::ArgumentParser &program);
void register_logger(argparse::ArgumentParser &program);
struct Config {
  std::string image_path;
  uint32_t mem_size;
  bool batch_mode;
  unsigned long itracer;
  bool mtracer;
  bool difftest;
  bool use_irb;
  unsigned long sz_irb;
  bool use_ftracer;
  std::string path_elf;
  NPCemu::DeviceSettings device_settings;
};

Config setup(argparse::ArgumentParser &program);