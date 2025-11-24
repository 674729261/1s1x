#include "Setup.h"
#include <Monitor/SingleMonitor.h>
#include <iostream>
#include <print>

using std::cerr;
using std::string;

int main(int argc, char *argv[]) {
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

  int result;

  SingleMonitor monitor(config.swap ? refemu : emu, config.mem_size,
                        config.image_path, config.device_settings,
                        config.batch_mode, config.itracer, config.mtracer,
                        config.sz_irb, config.use_ftracer, config.path_elf);

  if (config.difftest) {
    monitor.addReference(config.swap ? emu : refemu);
  }
  try {
    result = monitor.start();

  } catch (const std::exception &err) {
    std::println(std::cerr, "Error : {}", err.what());
    std::terminate();
  }

  spdlog::shutdown();
  println("ENd");
  return result;
}