#pragma once
#include <argparse/argparse.hpp>
#include <cstddef>
#include <print>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

void register_argparse(argparse::ArgumentParser &program);
void register_logger(argparse::ArgumentParser &program);
struct Config {
  std::string image_path;
  size_t mem_size;
  uint32_t base_memory;
  bool batch_mode;
  bool difftest;
  bool use_waveform;
  std::string waveform_file;
};

inline Config config;

Config setup(argparse::ArgumentParser &program);