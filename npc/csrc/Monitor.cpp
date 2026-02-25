#include "Expression/Expression.h"
#include "Setup.h"
#include "Simulate.h"
#include "my_utils.h"
#include <Monitor.h>
#include <cstdint>
#include <ctre/ctre.hpp>
#include <print>
using ctll::fixed_string;
using ctre::match;
static constexpr auto RE_NO_ARG = fixed_string{R"(\s*)"};
static constexpr auto RE_ONE_NUMBER =
    fixed_string{R"(\s*(0x[a-fA-F0-9]+|0[0-7]+|[1-9][0-9]*|0)\s*)"};
static constexpr auto RE_SCAN =
    fixed_string{R"(\s*(0x[a-fA-F0-9]+|0[0-7]+|[1-9][0-9]*|0)\s*(.*)\s*)"};
static constexpr auto RE_ONE_EXPR = fixed_string{R"(\s*(.*)\s*)"};

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
  if (auto [whole, num_str, expr_str] = match<RE_SCAN>(arg); whole) {
    auto n_of_w = to_number<unsigned long long>(num_str);
    auto result = Expr::Expression::create_expression(expr_str);
    if (!result.value.has_value())
      println("{}", result.error);
    else {
      uint32_t base_addr = result.value->last_value.value();
      if (!n_of_w.has_value())
        return CmdResult::INVALID_ARG;
      if (base_addr >= config.base_memory &&
          n_of_w.value() + base_addr < config.base_memory + config.mem_size) {
        println("Address \t Data");
        for (int i = 0; i < n_of_w; i++) {
          uint32_t addr = (base_addr + i * sizeof(uint32_t)) & ~0x3;
          println("{:08x}\t{:08x}", addr,
                  mem[(addr - config.base_memory) >> 2]);
        }
      } else {
        println("Invalid scan range [{:#010x},{:#010x})", base_addr & ~0x3,
                (n_of_w.value() + base_addr * sizeof(uint32_t)) & ~0x3);
        return CmdResult::INVALID_ARG;
      }
    }
  } else {
    return CmdResult::INVALID_ARG;
  }
  return CmdResult::OKAY;
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
CmdResult cmd_p(std::string_view arg) {
  if (auto [whole, expr_str] = match<RE_ONE_EXPR>(arg); whole) {
    auto result = Expr::Expression::create_expression(expr_str);
    if (!result.value.has_value())
      println("{}", result.error);
    else
      println("{0}\t{0:#010x}", result.value->last_value.value());
  } else {
    return CmdResult::INVALID_ARG;
  }
  return CmdResult::OKAY;
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