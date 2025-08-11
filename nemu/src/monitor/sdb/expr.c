/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "ST.h"
#include "common.h"
#include "sdb.h"
#include <assert.h>
#include <errno.h>
#include <isa.h>
#include <memory/vaddr.h>
#include <stdint.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <debug.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
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

static struct rule {
  const char *regex;
  int token_type;
  int catagry;
  int priority;
} rules[] = {

    {" +", TK_NOTYPE},                       // spaces
    {"\\+", '+', TK_CATAGORY_OPERATOR, 40},  // plus
    {"-", '-', TK_CATAGORY_OPERATOR, 40},    // minus
    {"\\*", '*', TK_CATAGORY_OPERATOR, 100}, // times
    {"\\/", '/', TK_CATAGORY_OPERATOR, 100}, // over
    {"%", '%', TK_CATAGORY_OPERATOR, 100},   // mod

    {"\\$\\w*[0-9]*", TK_REGISTER, TK_CATAGORY_OPERAND}, // register

    {"&&", TK_BOOL_AND, TK_CATAGORY_OPERATOR, 4},    // bool and
    {"\\|\\|", TK_BOOL_OR, TK_CATAGORY_OPERATOR, 3}, // bool or

    {"\\^", '^', TK_CATAGORY_OPERATOR, 6},    // xor
    {"&", '&', TK_CATAGORY_OPERATOR, 7},      // and
    {"\\|", '|', TK_CATAGORY_OPERATOR, 5},    // or
    {"~", '~', TK_CATAGORY_OPERATOR_SINGLE},  // inv
    {"<=", TK_LEQ, TK_CATAGORY_OPERATOR, 30}, // less or equal
    {">=", TK_GEQ, TK_CATAGORY_OPERATOR, 30}, // greater or equal
    {"<", '<', TK_CATAGORY_OPERATOR, 30},     // less
    {">", '>', TK_CATAGORY_OPERATOR, 30},     // greater
    {"==", TK_EQ, TK_CATAGORY_OPERATOR, 20},  // equal
    {"!=", TK_NEQ, TK_CATAGORY_OPERATOR, 20}, // not equal

    {"!", '!', TK_CATAGORY_OPERATOR_SINGLE}, // bool not

    {"(0[x,X])?[0-9,a-f,A-F]+", TK_NUMBER, TK_CATAGORY_OPERAND}, // a number
    {"\\(", '('}, // left parentheses
    {"\\)", ')'}, // right parentheses

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

static Token tokens[TOKEN_MAX_COUNT] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(const char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        const char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);

        if (substr_len >= TOKEN_SUBSTR_LEN) {
          printf("Token at position %d with len %d is too long\n", position,
                 substr_len);
          return false;
        }

        position += substr_len;

        switch (rules[i].token_type) {
        case TK_NOTYPE:
          break;
        case '+':
        case '-':
        case '*':

          if (nr_token == 0 || tokens[nr_token - 1].type == '(' ||
              tokens[nr_token - 1].catagry == TK_CATAGORY_OPERATOR ||
              tokens[nr_token - 1].catagry == TK_CATAGORY_OPERATOR_SINGLE) {
            Assert(nr_token != TOKEN_MAX_COUNT, "Too many tokens");
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            tokens[nr_token].str_sz = substr_len;
            tokens[nr_token].catagry = TK_CATAGORY_OPERATOR_SINGLE;
            tokens[nr_token].priority = 114514;
            nr_token++;
            break;
          }

        default:

          Assert(nr_token != TOKEN_MAX_COUNT, "Too many tokens");
          tokens[nr_token].type = rules[i].token_type;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          tokens[nr_token].str_sz = substr_len;
          tokens[nr_token].catagry = rules[i].catagry;
          tokens[nr_token].priority = rules[i].priority;
          nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int right_parentheses_pos[TOKEN_MAX_COUNT];

bool check_parentheses_legal() {
  int stack_parentheses[TOKEN_MAX_COUNT];
  int cnt_stack = 0;
  for (int i = 0; i < nr_token; i++) {
    tokens[i].layer = cnt_stack;
    if (tokens[i].type == '(') {
      stack_parentheses[cnt_stack++] = i;
    } else if (tokens[i].type == ')') {
      int pos_left_parentheses = stack_parentheses[--cnt_stack];
      if (cnt_stack < 0)
        return false;
      right_parentheses_pos[pos_left_parentheses] = i;
    }
  }
  return cnt_stack == 0;
}

const char *EVAL_ERROR_ILLEGAL_EXPR = "Expression is illegal.";
const char *EVAL_ERROR_LATGE_CONST = "Numeric constant id too large.";
const char *EVAL_ERROR_DIV_BY_ZERO = "Divided by zero";
const char *EVAL_ERROR_INVALID_ADDRESS = "Invalid address";
const char *EVAL_ERROR_INVALID_REGISTER = "Invalid register";

static const char *eval_error_flag;

int find_main_token(int p, int q, TokenST *st) {
  // 括号内的不选
  // 非运算符不选
  // 先选优先级低的
  // 先选靠右的
  for (int i = p; i <= q; i++)
    printf("%s", tokens[i].str);
  putchar('\n');
  int ret = query_ST(st, p, q);
  int cur_layer = tokens[p].layer;
  if (ret == -1 || cur_layer != tokens[ret].layer)
    return -1;
  return ret;
}

TokenST st;
long long eval(int p, int q, TokenST *st) {
  if (p > q) {
    eval_error_flag = EVAL_ERROR_ILLEGAL_EXPR;
    return -1;
  }

  if (p == q) {
    switch (tokens[p].type) {
    case TK_NUMBER: {
      errno = 0;
      long long result = strtoll(tokens[p].str, NULL, 0);
      if (errno != 0) {
        errno = 0;
        eval_error_flag = EVAL_ERROR_LATGE_CONST;
        return -1;
      }
      return result;
    }
    case TK_REGISTER: {
      bool success = true;
      word_t ret = isa_reg_str2val(tokens[q].str, &success);
      if (!success) {
        eval_error_flag = EVAL_ERROR_INVALID_REGISTER;
        return -1;
      }
      return ret;
    }
    default:
      eval_error_flag = EVAL_ERROR_ILLEGAL_EXPR;
      return -1;
    }
  }
  if (tokens[p].type == '(' && tokens[q].type == ')' &&
      right_parentheses_pos[p] == q)
    return eval(p + 1, q - 1, st);
  else {
    int main_token = find_main_token(p, q, st);

    if (main_token == -1) {
      // 处理一元运算符
      switch (tokens[p].type) {
      case '+':
        return eval(p + 1, q, st);
      case '-':
        return -eval(p + 1, q, st);
      case '~':
        return ~eval(p + 1, q, st);
      case '!':
        return !eval(p + 1, q, st);
      case '*': {
        long long value = eval(p + 1, q, st);
        if (value > (long long)UINT32_MAX || value < 0) {
          eval_error_flag = EVAL_ERROR_INVALID_ADDRESS;
          return -1;
        }
        word_t addr = (word_t)value;
        return vaddr_read(addr, 4);
      }
      case '$':
      default:
        Assert(0, "Failed to pick main token");
      }
    }
    // 处理二元运算符
    long long LHS = eval(p, main_token - 1, st);
    long long RHS = eval(main_token + 1, q, st);
    switch (tokens[main_token].type) {
    case '+':
      return LHS + RHS;
    case '-':
      return LHS - RHS;
    case '*':
      return LHS * RHS;
    case '/':
      if (RHS == 0) {
        eval_error_flag = EVAL_ERROR_DIV_BY_ZERO;
        return -1;
      }
      return LHS / RHS;
    case '%':
      if (RHS == 0) {
        eval_error_flag = EVAL_ERROR_DIV_BY_ZERO;
        return -1;
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
      eval_error_flag = EVAL_ERROR_ILLEGAL_EXPR;
      return -1;
    }
  }
  return 0;
}

long long expr(const char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  if (nr_token == 0) {
    printf("There is no expression.\n");
    *success = false;
    return 0;
  }
  if (check_parentheses_legal()) {

    TokenST st;
    init_ST(&st, nr_token, tokens);
    long long result = eval(0, nr_token - 1, &st);

    if (eval_error_flag) {
      puts(eval_error_flag);
      eval_error_flag = NULL;
      *success = false;
      return 0;
    }

    return result;
  }

  printf("Expression has illegal parentheses.\n");
  *success = false;
  return 0;
}
