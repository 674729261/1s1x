#include "Setup.h"
#include <Monitor/SingleMonitor.h>

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
  register_logger(program);

  auto config = setup(program);

  emu = make_shared<NPCemu>(config.mem_size, config.image_path,
                            config.device_settings);

  SingleMonitor monitor(emu, config.batch_mode, config.itracer, config.mtracer,
                        config.sz_irb, config.use_ftracer, config.path_elf);
  int result;
  if (config.difftest)
    monitor.addReference(nemu);
  try {
    result = monitor.start();
  } catch (const std::exception &err) {
    cerr << err.what() << std::endl;
    std::terminate();
  }
  emu = nullptr;
  spdlog::shutdown();
  return result;
}