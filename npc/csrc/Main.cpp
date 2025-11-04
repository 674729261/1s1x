#include "Setup.h"
#include <Monitor/SingleMonitor.h>
#include <iostream>
#include <print>

using std::cerr;
using std::make_shared;
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
#ifdef USE_REF_AS_MAIN
  SingleMonitor monitor(refemu, config.mem_size, config.image_path,
                        config.device_settings, config.batch_mode,
                        config.itracer, config.mtracer, config.sz_irb,
                        config.use_ftracer, config.path_elf);
#else
  SingleMonitor monitor(emu, config.mem_size, config.image_path,
                        config.device_settings, config.batch_mode,
                        config.itracer, config.mtracer, config.sz_irb,
                        config.use_ftracer, config.path_elf);
#endif

  if (config.difftest) {
#ifdef USE_REF_AS_MAIN
    monitor.addReference(emu);
#else
    monitor.addReference(refemu);
#endif
  }
  try {
    result = monitor.start();

  } catch (const std::exception &err) {
    std::println(std::cerr, "Error : {}", err.what());
    std::terminate();
  }

  spdlog::shutdown();
  return result;
}