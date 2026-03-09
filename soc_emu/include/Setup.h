#pragma once

#include <argparse/argparse.hpp>
#include <print>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

void register_argparse(argparse::ArgumentParser &program);
void register_logger(argparse::ArgumentParser &program);
struct Config {
  std::string image_path;
  bool batch_mode;

  unsigned nr_cacheline_words_2pow;
  unsigned nr_cachelines_2pow;
};

Config setup(argparse::ArgumentParser &program);