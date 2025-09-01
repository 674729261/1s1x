#include "Expression/Expression.hpp"
#include "utils.h"
#include <iterator>
#include <optional>
#include <print>
#include <regex>
#include <string>
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
        case TK_NUMBER:
          tokens.emplace_back(
              std::string(expr.begin() + pos, expr.begin() + pos + mm.length()),
              tokenTypes[i].type, tokenTypes[i].catagory,
              tokenTypes[i].priotity);
          auto parsed_value = to_number<long long>(tokens.back().display);
          if (!parsed_value.has_value())
            return std::nullopt;
          tokens.back().data.value = parsed_value.value();
          break;
        }
        pos += mm.length();
        break;
      }
      i++;
    }
    if (!matched) {
      std::println("Invalid token:");
      std::println("{}", expr);
      while (pos--)
        std::print(" ");
      std::println("^");
      return std::nullopt;
    }
  }
}
