#include "Simulate.h"
#include "my_utils.h"
#include <Monitor.h>
#include <ctre/ctre.hpp>
#include <print>
using ctll::fixed_string;
using ctre::match;
static constexpr auto RE_NO_ARG = fixed_string{R"(\s*)"};
static constexpr auto RE_ONE_NUMBER = fixed_string{
    R"(\s*(0x[a-fA-F1-9][a-fA-F0-9]*|0[1-7][0-7]*|[1-9][0-9]*|0)\s*)"};
static constexpr auto RE_TWO_NUMBER = fixed_string{
    R"(\s*(0x[a-fA-F1-9][a-fA-F0-9]*|0[1-7][0-7]*|[1-9][0-9]*|0)\s*(0x[a-fA-F1-9][a-fA-F0-9]*|0[1-7][0-7]*|[1-9][0-9]*|0)\s*)"};
static constexpr auto RE_ONE_EXPR = fixed_string{R"(\s*.*\s*)"};

CmdResult cmd_c(std::string_view arg) {
  if (match<RE_NO_ARG>(arg)) {
    run(-1);
    return CmdResult::OKAY;
  } else {
    return CmdResult::INVALID_ARG;
  }
}
CmdResult cmd_si(std::string_view arg) {
  if (match<RE_NO_ARG>(arg)) {
    run(1);
    return CmdResult::OKAY;
  } else if (auto [whole, num_str] = match<RE_ONE_NUMBER>(arg); whole) {
    auto value = to_number<unsigned long long>(num_str);
    if (value.has_value()) {
      unsigned long long steps = value.value();
      run(steps);
    } else {
      return CmdResult::INVALID_ARG;
    }
    return CmdResult::OKAY;
  } else {
    return CmdResult::INVALID_ARG;
  }
}

CmdResult cmd_x(std::string_view arg) {
  if (auto [whole, num_str_1, num_str_2] = match<RE_TWO_NUMBER>(arg); whole) {
    auto n_of_w = to_number<unsigned long long>(num_str_1);
    auto base_addr = to_number<unsigned long long>(num_str_2);
    if (n_of_w.has_value() && base_addr.has_value()) {
      todo("x");
    } else {
      return CmdResult::INVALID_ARG;
    }
    return CmdResult::OKAY;
  } else {
    return CmdResult::INVALID_ARG;
  }
}

CmdResult cmd_q(std::string_view arg) {
  if (match<RE_NO_ARG>(arg)) {
    quit.store(true);
    return CmdResult::OKAY;
  } else {
    return CmdResult::INVALID_ARG;
  }
}

CmdResult cmd_li(std::string_view arg) {
  if (match<R"(\s*r\s*)">(arg)) {
    dut->print_all_gpr();
    return CmdResult::OKAY;
  } else if (match<R"(\s*w\s*)">(arg)) {
    todo("li w");
  } else {
    return CmdResult::INVALID_ARG;
  }
}

CmdResult cmd_help(std::string_view arg) {
  if (match<RE_NO_ARG>(arg)) {
    for (int i = 0; i < NR_CMD; i++) {
      std::println("{}\t{}", cmd_list[i].command, cmd_list[i].help);
    }
  } else {
    auto [whole, cmd] = match<R"(\s*([a-z]+)\s*)">(arg);
    for (int i = 0; i < NR_CMD; i++) {
      if (cmd.to_view() == cmd_list[i].command) {
        {
          std::println("{}\t{}", cmd_list[i].command, cmd_list[i].help);
          return CmdResult::OKAY;
        }
      }
    }
    println("No such command. All command list:");
    for (int i = 0; i < NR_CMD; i++) {
      std::println("{}\t{}", cmd_list[i].command, cmd_list[i].help);
    }
  }
  return CmdResult::OKAY;
}