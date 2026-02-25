#pragma once
#include <cstdint>
#include <ctre/ctre.hpp>
#include <functional>
#include <string_view>
namespace Expr {
using namespace ctre::literals;
using TokenizorType = std::function<std::string_view(std::string_view)>;
TokenizorType make_token_re(auto re) {
  return
      [re](std::string_view str) -> std::string_view { return re.search(str); };
}

enum TokenID : uint8_t {
  TK_NULL = 0,
  TK_NUM = 128,
  TK_EQ,
  TK_NEQ,
  TK_BOOL_AND,
  TK_BOOL_OR,
  TK_GE,
  TK_LE,
  TK_REG
};
enum class Catagory { OPERATOR_2, OPERATOR_1, OPERAND };
struct Token {
  int type;
  Catagory cata;
  uint32_t data;
};
const struct {
  TokenizorType tokenizor;
  int id;
  int priority = -1;
  Catagory cata = Catagory::OPERATOR_2;
} token_types[] = {
    {make_token_re(R"(^\s+)"_ctre), TK_NULL},
    {make_token_re(
         R"(^0x[a-fA-F1-9][a-fA-F0-9]*|0[1-7][0-7]*|[1-9][0-9]*|0)"_ctre),
     TK_NUM, -1, Catagory::OPERAND},
    {make_token_re(R"(^\$[a-z]{0,2}[0-9]?)"_ctre), TK_REG, -1,
     Catagory::OPERAND},
    {make_token_re(R"(^\+)"_ctre), '+', 40},
    {make_token_re(R"(^-)"_ctre), '-', 40},
    {make_token_re(R"(^\*)"_ctre), '*', 100},
    {make_token_re(R"(^/)"_ctre), '/', 100},
    {make_token_re(R"(^%)"_ctre), '%', 100},
    {make_token_re(R"(^==)"_ctre), TK_EQ, 20},
    {make_token_re(R"(^!=)"_ctre), TK_NEQ, 20},
    {make_token_re(R"(^>=)"_ctre), TK_GE, 30},
    {make_token_re(R"(^<=)"_ctre), TK_LE, 30},
    {make_token_re(R"(^>)"_ctre), '>', 30},
    {make_token_re(R"(^<)"_ctre), '<', 30},
    {make_token_re(R"(^!)"_ctre), '!', -1, Catagory::OPERATOR_1},
    {make_token_re(R"(^&&)"_ctre), TK_BOOL_AND, 4},
    {make_token_re(R"(^\|\|)"_ctre), TK_BOOL_OR, 3},
    {make_token_re(R"(^~)"_ctre), '~', -1, Catagory::OPERATOR_1},
    {make_token_re(R"(^&)"_ctre), '&', 7},
    {make_token_re(R"(^\|)"_ctre), '|', 5},
    {make_token_re(R"(^\^)"_ctre), '^', 6},
    {make_token_re(R"(^\()"_ctre), '(', -1, Catagory::OPERATOR_1},
    {make_token_re(R"(^\))"_ctre), ')', -1, Catagory::OPERAND}};
constexpr size_t NR_TOKEN_TYPES = sizeof(token_types) / sizeof(token_types[0]);
} // namespace Expr