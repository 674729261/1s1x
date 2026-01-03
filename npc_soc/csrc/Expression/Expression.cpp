#include "Device/Device.h"
#include <Expression/Expression.h>
#include <Simulators/RISCV32.h>
#include <format>
#include <my_utils.h>
#include <optional>
#include <print>
#include <regex>
#include <spdlog/spdlog.h>
#include <stack>
#include <string>
#include <vector>

using std::regex, std::smatch, std::regex_search, std::regex_search;

struct Token {
  regex regex_expr;
  int type;
  int catagory;
  int priotity;
};

enum TokenType {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_NUMBER,
  TK_REGISTER,
  TK_LEQ,
  TK_GEQ,
  TK_BOOL_AND,
  TK_BOOL_OR

};
enum TokenCatagory {
  TK_CATAGORY_OTHER = 0,
  TK_CATAGORY_OPERATOR,
  TK_CATAGORY_OPERATOR_SINGLE,
  TK_CATAGORY_OPERAND
};

static Token tokenTypes[] = {
    {regex(" +"), TK_NOTYPE},                       // spaces
    {regex("\\+"), '+', TK_CATAGORY_OPERATOR, 40},  // plus
    {regex("-"), '-', TK_CATAGORY_OPERATOR, 40},    // minus
    {regex("\\*"), '*', TK_CATAGORY_OPERATOR, 100}, // times
    {regex("\\/"), '/', TK_CATAGORY_OPERATOR, 100}, // over
    {regex("%"), '%', TK_CATAGORY_OPERATOR, 100},   // mod

    {regex("\\$\\w*[0-9]*"), TK_REGISTER, TK_CATAGORY_OPERAND}, // register

    {regex("&&"), TK_BOOL_AND, TK_CATAGORY_OPERATOR, 4},    // bool and
    {regex("\\|\\|"), TK_BOOL_OR, TK_CATAGORY_OPERATOR, 3}, // bool or

    {regex("\\^"), '^', TK_CATAGORY_OPERATOR, 6},    // xor
    {regex("&"), '&', TK_CATAGORY_OPERATOR, 7},      // and
    {regex("\\|"), '|', TK_CATAGORY_OPERATOR, 5},    // or
    {regex("~"), '~', TK_CATAGORY_OPERATOR_SINGLE},  // inv
    {regex("<="), TK_LEQ, TK_CATAGORY_OPERATOR, 30}, // less or equal
    {regex(">="), TK_GEQ, TK_CATAGORY_OPERATOR, 30}, // greater or equal
    {regex("<"), '<', TK_CATAGORY_OPERATOR, 30},     // less
    {regex(">"), '>', TK_CATAGORY_OPERATOR, 30},     // greater
    {regex("=="), TK_EQ, TK_CATAGORY_OPERATOR, 20},  // equal
    {regex("!="), TK_NEQ, TK_CATAGORY_OPERATOR, 20}, // not equal

    {regex("!"), '!', TK_CATAGORY_OPERATOR_SINGLE}, // bool not

    {regex("(0[x,X])?[0-9,a-f,A-F]+"), TK_NUMBER,
     TK_CATAGORY_OPERAND}, // a number
    {regex("\\("), '('},   // left parentheses
    {regex("\\)"), ')'},   // right parentheses
};
std::optional<Expression>
Expression::generateExpression(std::string_view expr) {
  std::vector<Expression::OneToken> tokens;
  size_t pos = 0;
  while (pos < expr.size()) {
    using SVMatchResults = std::match_results<std::string_view::const_iterator>;
    SVMatchResults mm;
    bool matched = false;

    for (int i = 0; auto &token_type : tokenTypes) {

      auto &re = token_type.regex_expr;
      std::string_view rest(expr.begin() + pos);
      if (regex_search(rest.begin(), rest.end(), mm, re,
                       std::regex_constants::match_continuous)) {

        matched = true;
        switch (tokenTypes[i].type) {
        case TK_NOTYPE:
          break;
        case TK_NUMBER: {
          tokens.emplace_back(
              std::string(expr.begin() + pos, expr.begin() + pos + mm.length()),
              tokenTypes[i].type, tokenTypes[i].catagory,
              tokenTypes[i].priotity);
          auto parsed_value = to_number<uint32_t>(tokens.back().display);
          if (!parsed_value.has_value())
            return std::nullopt;
          tokens.back().data.value = parsed_value.value();
        } break;
        case TK_REGISTER: {
          tokens.emplace_back(
              std::string(expr.begin() + pos, expr.begin() + pos + mm.length()),
              tokenTypes[i].type, tokenTypes[i].catagory,
              tokenTypes[i].priotity);
          int gpr_id = RISCV32::getGPRIDfromName(tokens.back().display);
          if (gpr_id < 0) {
            println("Invalid gpr name : {}", tokens.back().display);
            return std::nullopt;
          }
          tokens.back().data.regid = gpr_id;
        } break;
        case '+':
        case '-':
        case '*': {
          if (tokens.size() == 0 || tokens.back().type == '(' ||
              tokens.back().catagory == TK_CATAGORY_OPERATOR ||
              tokens.back().catagory == TK_CATAGORY_OPERATOR_SINGLE) {
            tokens.emplace_back(std::string(expr.begin() + pos,
                                            expr.begin() + pos + mm.length()),
                                tokenTypes[i].type, TK_CATAGORY_OPERATOR_SINGLE,
                                114514);
            break;
          }
        } // fallthrough
        default:
          tokens.emplace_back(
              std::string(expr.begin() + pos, expr.begin() + pos + mm.length()),
              tokenTypes[i].type, tokenTypes[i].catagory,
              tokenTypes[i].priotity);
        }
        pos += mm.length();
        break;
      }
      i++;
    }
    if (!matched) {
      println("Invalid token:");
      println("{}", expr);
      while (pos--)
        print(" ");
      println("^");
      return std::nullopt;
    }
  }

  // check parentheses
  std::vector<int> parentheses(tokens.size());
  std::stack<int> stack_parentheses;
  for (int i = 0; i < tokens.size(); i++) {
    if (tokens[i].type == '(')
      stack_parentheses.push(i);
    else if (tokens[i].type == ')') {
      if (stack_parentheses.empty()) {
        println("Unpaired parentheses");
        return std::nullopt;
      }
      parentheses[stack_parentheses.top()] = i;
      stack_parentheses.pop();
    }
  }
  if (!stack_parentheses.empty()) {
    println("Unpaired parentheses");
    return std::nullopt;
  }
  if (tokens.empty()) {
    println("Invalid expression");
    return std::nullopt;
  }
  Expression expression;
  expression.tokens = std::move(tokens);
  expression.parentheses = std::move(parentheses);
  return expression;
}

uint32_t Expression::eval(RISCV32 &dut, Devices &devices) const {
  return eval_sub(dut, devices, 0, tokens.size() - 1);
}

uint32_t Expression::eval_sub(RISCV32 &dut, Devices &devices, int l,
                              int r) const {
  if (l > r)
    log_and_throw<EvaluationError>("Invalid expression {}", stringify());
  if (l == r) {
    if (tokens[l].type == TK_NUMBER)
      return tokens[l].data.value;
    if (tokens[l].type == TK_REGISTER)
      return dut.getGPR(tokens[l].data.regid);
    spdlog::error("Invalid expression {}", stringify());
    return -1;
  }
  if (parentheses[l] == r)
    return eval_sub(dut, devices, l + 1, r - 1);
  int pos_main = main_token(dut, l, r);
  if (pos_main == -1) {
    switch (tokens[l].type) {
    case '+':
      return eval_sub(dut, devices, l + 1, r);
    case '-':
      return -eval_sub(dut, devices, l + 1, r);
    case '~':
      return ~eval_sub(dut, devices, l + 1, r);
    case '!':
      return !eval_sub(dut, devices, l + 1, r);
    case '*':
      return devices.readMemory(eval_sub(dut, devices, l + 1, r));
    default:
      log_and_throw<EvaluationError>("Invalid expression : {}", stringify());
    }
  }
  uint32_t LHS = eval_sub(dut, devices, l, pos_main - 1);
  uint32_t RHS = eval_sub(dut, devices, pos_main + 1, r);
  switch (tokens[pos_main].type) {
  case '+':
    return LHS + RHS;
  case '-':
    return LHS - RHS;
  case '*':
    return LHS * RHS;
  case '/':
    if (RHS == 0) {
      log_and_throw<EvaluationError>("Division by zero : {}", stringify());
    }
    return LHS / RHS;
  case '%':
    if (RHS == 0) {
      log_and_throw<EvaluationError>("Division by zero : {}", stringify());
    }
    return LHS % RHS;
  case '^':
    return LHS ^ RHS;
  case '&':
    return LHS & RHS;
  case '|':
    return LHS | RHS;
  case TK_EQ:
    return LHS == RHS;
  case TK_NEQ:
    return LHS != RHS;
  case TK_LEQ:
    return LHS <= RHS;
  case TK_GEQ:
    return LHS >= RHS;
  case '<':
    return LHS < RHS;
  case '>':
    return LHS > RHS;
  case TK_BOOL_AND:
    return LHS && RHS;
  case TK_BOOL_OR:
    return LHS || RHS;
  default:
    log_and_throw<EvaluationError>("Invalid expression : {}", stringify());
  }
}
int Expression::main_token(RISCV32 &, int l, int r) const {
  int ret = -1, mn = 99999;

  for (int i = l; i <= r; i++) {
    if (tokens[i].type == '(') {
      i = parentheses[i];
      continue;
    }
    if (tokens[i].catagory == TK_CATAGORY_OPERATOR &&
        tokens[i].priority <= mn) {
      mn = tokens[i].priority;
      ret = i;
    }
  }
  return ret;
}
std::string Expression::stringify() const {
  std::string ret;
  for (const auto &tk : tokens)
    ret += std::format("{}", tk.display);
  return ret;
}

optional<uint32_t> Expression::evalExpression(RISCV32 &dut, Devices &devices,
                                              std::string_view expr) {
  auto e = generateExpression(expr);
  uint32_t value;
  if (!e.has_value())
    return std::nullopt;

  try {
    value = e->eval(dut, devices);
  } catch (EvaluationError e) {
    println("{}", e.what());
    println("Evaluation failed", e.what());
    return std::nullopt;
  }

  return value;
}