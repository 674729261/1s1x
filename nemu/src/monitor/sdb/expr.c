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

#include "common.h"
#include <assert.h>
#include <errno.h>
#include <isa.h>
#include <stdint.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <debug.h>
#include <regex.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define TOKEN_SUBSTR_LEN 32
#define TOKEN_MAX_COUNT 64

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_NUMBER

};

enum { TK_CATAGORY_OTHER = 0, TK_CATAGORY_OPERATOR, TK_CATAGORY_OPERAND };

static struct rule {
  const char *regex;
  int token_type;
  int catagry;
  int priority;
} rules[] = {

    {" +", TK_NOTYPE},                                           // spaces
    {"\\+", '+', TK_CATAGORY_OPERATOR, 1},                       // plus
    {"-", '-', TK_CATAGORY_OPERATOR, 1},                         // minus
    {"\\*", '*', TK_CATAGORY_OPERATOR, 2},                       // times
    {"\\/", '/', TK_CATAGORY_OPERATOR, 2},                       // over
    {"==", TK_EQ, TK_CATAGORY_OPERATOR, 0},                      // equal
    {"!=", TK_NEQ, TK_CATAGORY_OPERATOR, 0},                     // not equal
    {"(0[x,X])?[0-9,a-f,A-F]+", TK_NUMBER, TK_CATAGORY_OPERAND}, // a number
    {"\\(", '('},                                                // left brace
    {"\\)", ')'},                                                // right brace
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

typedef struct token {
  int type;
  char str[TOKEN_SUBSTR_LEN];
  int str_sz;
  int catagry;
  int priority;
} Token;

static Token tokens[TOKEN_MAX_COUNT] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);

        Assert(substr_len < TOKEN_SUBSTR_LEN,
               "Token at position %d with len %d is too long", position,
               substr_len);

        position += substr_len;

        switch (rules[i].token_type) {
        case TK_NOTYPE:
          break;
        default:

          if (nr_token == 0 || tokens[nr_token - 1].type == '+' ||
              tokens[nr_token - 1].type == '-') {
            Assert(nr_token != TOKEN_MAX_COUNT, "Too many tokens");
            tokens[nr_token].type = TK_NUMBER;
            tokens[nr_token].str[0] = '0';
            tokens[nr_token].str[1] = '\0';
            tokens[nr_token].str_sz = 1;
            tokens[nr_token].catagry = TK_CATAGORY_OPERAND;
            tokens[nr_token].priority = 114514;
            nr_token++;
          }
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

static int right_brace_pos[TOKEN_MAX_COUNT];

bool check_brace_legal() {
  int stack_brace[TOKEN_MAX_COUNT];
  int cnt_stack = 0;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '(')
      stack_brace[cnt_stack++] = i;
    else if (tokens[i].type == ')') {
      int pos_left_brace = stack_brace[--cnt_stack];
      if (cnt_stack < 0)
        return false;
      right_brace_pos[pos_left_brace] = i;
    }
  }
  return true;
}

const char *EVAL_ERROR_ILLEGAL_EXPR = "Expression is illegal.";
const char *EVAL_ERROR_LATGE_CONST = "Numeric constant id too large.";
const char *EVAL_ERROR_DIV_BY_ZERO = "Divided by zero";

static const char *eval_error_flag;

int find_main_token(int p, int q) {
  // 括号内的不选
  // 非运算符不选
  // 先选优先级低的
  // 先选靠右的
  int cnt_brace = 0, selected = -1, lowest_prior = 114514;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(')
      cnt_brace++;
    else if (tokens[i].type == ')')
      cnt_brace--;
    else if (tokens[i].catagry == TK_CATAGORY_OPERATOR && cnt_brace == 0) {
      if (lowest_prior >= tokens[i].priority) {
        lowest_prior = tokens[i].priority;
        selected = i;
      }
    }
  }
  return selected;
}

long long eval(int p, int q) {
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
    default:
      eval_error_flag = EVAL_ERROR_ILLEGAL_EXPR;
      return -1;
    }
  }

  if (tokens[p].type == '(' && tokens[q].type == ')' && right_brace_pos[p] == q)
    return eval(p + 1, q - 1);
  else {
    int main_token = find_main_token(p, q);
    Assert(main_token != -1, "Failed to pick main token");
    long long LHS = eval(p, main_token - 1);
    long long RHS = eval(main_token + 1, q);
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
    case TK_EQ:
      return LHS == RHS;
    case TK_NEQ:
      return LHS != RHS;
    default:
      eval_error_flag = EVAL_ERROR_ILLEGAL_EXPR;
      return -1;
    }
  }
  return 0;
}

long long expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (check_brace_legal()) {
    long long result = eval(0, nr_token - 1);

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
