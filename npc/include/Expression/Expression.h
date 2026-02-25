#pragma once
#include "DUT.h"
#include "Simulate.h"
#include "my_utils.h"
#include "spdlog/spdlog.h"
#include <Expression/ExpressionToken.h>
#include <SDL2/SDL_stdinc.h>
#include <cmath>
#include <cstdint>
#include <optional>
#include <print>
#include <stack>
#include <string>
#include <string_view>
#include <vector>
namespace Expr {

struct Expression {

  Expression(const Expression &) = default;
  Expression(Expression &&) noexcept = default;
  std::vector<Token> nodes;
  std::string display;
  static Result<Expression> create_expression(std::string_view expr_str);
  uint32_t last_value;
  Result<uint32_t> eval();

private:
  Expression() = default;
};

inline Result<std::vector<Token>>
build_expr_tree(const std::vector<Token> &vtk) {
  std::vector<Token> ret;
  ret.reserve(vtk.size());
  std::stack<Token> stk_node;
  for (const Token &tk : vtk) {
    const auto tt_cur = token_types[tk.type];
    if (tt_cur.id == '(') {
      stk_node.push(tk);
    } else if (tt_cur.id == ')') {
      while (!stk_node.empty() && token_types[stk_node.top().type].id != '(') {
        ret.push_back(stk_node.top());
        stk_node.pop();
      }
      if (stk_node.empty()) {
        return {std::nullopt, "Invalid expression : unpaired bracket"};
      } else {
        // paired bracket
        stk_node.pop();
      }
    } else {
      switch (token_types[tk.type].cata) {
      case Catagory::OPERAND:
        ret.push_back(tk);
        break;
      case Catagory::OPERATOR_2: {
        while (!stk_node.empty() &&
               token_types[stk_node.top().type].priority <= tt_cur.priority) {
          ret.push_back(stk_node.top());
          stk_node.pop();
        }
        stk_node.push(tk);
        break;
      }
      case Catagory::OPERATOR_1:
        stk_node.push(tk);
        break;
      }
    }
  }
  while (!stk_node.empty()) {
    if (token_types[stk_node.top().type].id == '(')
      return {std::nullopt, "Invalid expression : unclosed bracket"};
    ret.push_back(stk_node.top());
    stk_node.pop();
  }
  return {ret, ""};
}

inline Result<Expression>
Expression::create_expression(std::string_view expr_str) {
  bool prev_is_operator = true;
  int cur_pos = 0;
  Expression ret;
  std::vector<Token> token_seq;
  while (cur_pos < expr_str.length()) {
    std::string_view cur_substr = expr_str.substr(cur_pos);
    int which = 0;
    std::string_view result;
    for (; which < NR_TOKEN_TYPES; which++) {
      const auto &tt = token_types[which];
      result = tt.tokenizor(cur_substr);
      if (result.length() > 0)
        break;
    }
    if (which == NR_TOKEN_TYPES) {
      return {std::nullopt,
              std::format("Unknown token at pos {} : {}", cur_pos, cur_substr)};
    }
    const auto &tt = token_types[which];
    if (tt.id == TK_NULL) {
      cur_pos += result.length();
      continue;
    }

    Token cur_token = {which, token_types[which].cata, 0};
    if (tt.id == TK_NUM) {
      auto parse_num = to_number<uint32_t>(result);
      if (!parse_num.has_value())
        return {std::nullopt, std::format("Invalid number: {}", result)};
      cur_token.data = parse_num.value();
    } else if (tt.id == TK_REG) {
      if (result == "$0")
        cur_token.data = 0;
      else if (result == "$pc") {
        cur_token.data = 32;
      } else {
        std::string_view reg_name = result.substr(1);
        bool found = false;
        for (int i = 1; i < 32; i++) {
          if (reg_name == gpr_names[i]) {
            cur_token.data = i;
            found = true;
            break;
          }
        }
        if (!found) {
          return {std::nullopt, std::format("Invalid gpr name : {}", result)};
        }
      }
    }

    if ((tt.id == '-' || tt.id == '+' || tt.id == '*') && prev_is_operator) {
      cur_token.cata = Catagory::OPERATOR_1;
    }
    if ((prev_is_operator && tt.cata == Catagory::OPERATOR_2) ||
        (!prev_is_operator && tt.cata != Catagory::OPERAND)) {
      return {std::nullopt,
              std::format("Invalid token '{}' at pos {}", result, cur_pos)};
    }
    prev_is_operator = (tt.cata != Catagory::OPERAND);
    ret.display += result;
    token_seq.push_back(cur_token);
    cur_pos += result.length();
  }
  spdlog::debug("{}", token_seq.size());
  auto suf = build_expr_tree(token_seq);
  if (!suf.value) {
    return {std::nullopt, suf.error};
  }

  ret.nodes = std::move(suf.value.value());
  auto val = ret.eval();
  if (!val.value) {
    return {std::nullopt, val.error};
  }
  ret.last_value = val.value.value();
  return {ret};
}
inline Result<uint32_t> Expression::eval() {
  std::stack<uint32_t> stk_calc;
  for (const Token &tk : nodes) {
    const auto &tt = token_types[tk.type];
    switch (tt.cata) {
    case Catagory::OPERAND:
      switch (tt.id) {
      case TK_NUM:
        stk_calc.push(tk.data);
        break;
      case TK_REG:
        if (tk.data == 32)
          stk_calc.push(dut->getPC());
        else
          stk_calc.push(dut->getGPR(tk.data));
        break;
      default:
        return {std::nullopt, "Unknown unary operator"};
      }
      break;
    case Catagory::OPERATOR_2: {
      if (stk_calc.size() < 2)
        return {std::nullopt, "Invalid expression : insufficient operands"};
      uint32_t rhs = stk_calc.top();
      stk_calc.pop();
      uint32_t lhs = stk_calc.top();
      stk_calc.pop();
      switch (tt.id) {
      case '+':
        stk_calc.push(lhs + rhs);
        break;
      case '-':
        stk_calc.push(lhs - rhs);
        break;
      case '*':
        stk_calc.push(lhs * rhs);
        break;
      case '/':
        if (rhs == 0)
          return {std::nullopt, "Evaluation error : division by zero"};
        stk_calc.push(lhs / rhs);
        break;
      case '%':
        if (rhs == 0)
          return {std::nullopt, "Evaluation error : division by zero"};
        stk_calc.push(lhs % rhs);
        break;
      default:
        return {std::nullopt, "Unknown operator"};
      }
      break;
    }
    case Catagory::OPERATOR_1:
      todo("unary operator");
      break;
    }
  }
  if (stk_calc.size() > 1)
    return {std::nullopt, "Invalid expression : insufficient operators"};
  if (stk_calc.empty())
    return {std::nullopt, "Invalid expression : empty expression"};
  return {stk_calc.top(), ""};
}

} // namespace Expr
