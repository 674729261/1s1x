#pragma once
#include "my_utils.h"
#include <Expression/ExpressionToken.h>
#include <SDL2/SDL_stdinc.h>
#include <cstdint>
#include <optional>
#include <print>
#include <stack>
#include <string>
#include <string_view>
#include <vector>
namespace Expr {
struct Token {
  int type;
  uint32_t data;
};
struct Expression {

  Expression(const Expression &) = default;
  Expression(Expression &&) noexcept = default;
  struct Node {
    int LHS;
    int RHS;
    Token token;
    uint32_t eval();
  };
  std::vector<Node> nodes;
  std::string display;
  static std::optional<Expression> create_expression(std::string_view expr_str);

private:
  Expression() = default;
};
inline std::optional<Expression>
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
      println("Unknown token at pos {} : {}", cur_pos, cur_substr);
      return std::nullopt;
    }
    const auto &tt = token_types[which];
    if (tt.id == TK_NULL)
      continue;

    Token cur_token = {which, 0};
    if (tt.id == TK_NUM) {
      auto parse_num = to_number<uint32_t>(result);
      if (!parse_num.has_value())
        return std::nullopt;
      cur_token.data = parse_num.value();
    }

    prev_is_operator = (tt.cata == Catagory::OPERAND);
    ret.display += result;
  }

  std::stack<Token> stk_node;

  return ret;
}

inline uint32_t Expression::Node::eval() {}
} // namespace Expr
