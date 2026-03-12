#pragma once

#include "../Simulators/RISCV32.h"
#include "Device/Device.h"
#include <optional>
#include <string>
#include <string_view>
#include <vector>
class Expression {
public:
  static std::optional<Expression> generateExpression(std::string_view expr);
  static std::optional<uint32_t> evalExpression(RISCV32 &dut, Devices &devices,
                                                std::string_view expr);
  Expression(const Expression &) = default;
  Expression(Expression &&) noexcept = default;

  uint32_t eval(RISCV32 &, Devices &) const;
  std::string stringify() const;

protected:
  Expression() = default;

private:
  uint32_t eval_sub(RISCV32 &, Devices &, int l, int r) const;
  int main_token(RISCV32 &, int l, int r) const;
  struct OneToken {
    std::string display;
    int type;
    int catagory;
    int priority;
    union {
      uint32_t value;
      int regid;
    } data;
  };

  std::vector<OneToken> tokens;
  std::vector<int> parentheses;
};