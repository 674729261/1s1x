
#include "Device/Device.h"
#include "spdlog/spdlog.h"
#include <Args.h>
#include <Mem.h>
#include <Monitor.h>
#include <Ref.h>
#include <SDL2/SDL_audio.h>
#include <Vnpc_top.h>
#include <Vnpc_top___024root.h>
#include <exception>
#include <iostream>
#include <print>
#include <verilated.h>
#include <verilated_vcd_c.h>
using std::string;

int main(int argc, char *argv[]) {
  config = process_args(argc, argv);
  int return_value = -1;
  try {
    return_value = simulate(argc, argv);
  } catch (const std::exception &e) {
    std::println(std::cerr, "Error : {}", e.what());
    if (device_thread) {
      device_thread->request_stop();
      // device_thread->join();
    }
  }
  spdlog::shutdown();

  return return_value;
}