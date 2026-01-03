#include "Setup.h"
#include <Monitor/SingleMonitor.h>
#include <iostream>
#include <print>

using std::cerr;
using std::string;

int main(int argc, char *argv[]) {
  Verilated::commandArgs(argc, argv);
  argparse::ArgumentParser program("NPCemu");
  register_argparse(program);
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    cerr << program;
    std::terminate();
  }
  Config config;
  try {
    register_logger(program);
    config = setup(program);
  } catch (const std::exception &err) {
    std::println(std::cerr, "Error : {}", err.what());
    std::terminate();
  }

  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vour *top = new Vour{contextp};

  int result;

  spdlog::shutdown();

  return result;
}