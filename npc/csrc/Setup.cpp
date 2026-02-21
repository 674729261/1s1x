#include "Setup.h"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <cstdint>
using std::string;

void register_argparse(argparse::ArgumentParser &program) {
  program.add_argument("-i", "--image")
      .help("The program image file")
      .nargs(1)
      .required();
  program.add_argument("-w", "--waveform").help("Waveform file name").nargs(1);
  program.add_argument("-l", "--log").help("Path to log file");
  program.add_argument("-d", "--difftest")
      .help("Use NEMUemu as differential test")
      .flag();
  program.add_argument("-m", "--mem_size")
      .help("Capacity of memory in words")
      .scan<'u', size_t>();
  program.add_argument("--mem_base")
      .help("Address base of memory")
      .scan<'u', uint32_t>();
  program.add_argument("-b", "--batch").help("Use batch mode").flag();
}

void register_logger(argparse::ArgumentParser &program) {
  bool provided_logfile = program.is_used("--log");
  config.use_waveform = program.is_used("--waveform");
  if (config.use_waveform)
    config.waveform_file = program.get("--waveform");

  if (provided_logfile) {
    string log_path = program.get("--log");
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern(
        "[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%^%l%$] - %v");

    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, true);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%^%l%$] - %v");
    spdlog::logger logger("multi_logger", {console_sink, file_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
    spdlog::info("Logging to file : {}", log_path);
  }

  spdlog::flush_every(std::chrono::seconds(5));
  spdlog::flush_on(spdlog::level::warn);
}

Config setup(argparse::ArgumentParser &program) {

  Config ret = {};
  ret.mem_size = program.get<size_t>("--memsize");
  ret.image_path = program.get("--image");
  ret.batch_mode = program.get<bool>("--batch");
  spdlog::info("Image path  : {}", ret.image_path);
  ret.difftest = program.get<bool>("--difftest");
  if (ret.difftest)
    spdlog::info("Using difftest");

  return ret;
}
