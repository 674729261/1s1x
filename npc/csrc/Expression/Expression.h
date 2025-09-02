#pragma once

#include "../Simulators/RISCV32.h"
#include <optional>
#include <string>
#include <string_view>
#include <vector>
class Expression {
public:
  static std::optional<Expression> generateExpression(std::string_view expr);
  Expression(const Expression &) = default;
  Expression(Expression &&) noexcept = default;

  long long eval(RISCV32 &);
  std::string stringify();

private:
  Expression() = default;
  long long eval_sub(RISCV32 &, int l, int r);
  int main_token(RISCV32 &, int l, int r);
  struct OneToken {
    std::string display;
    int type;
    int catagory;
    int priority;
    union {
      unsigned long long value;
      int regid;
    } data;
  };

  std::vector<OneToken> tokens;
  std::vector<int> parentheses;
};