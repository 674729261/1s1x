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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

#define TOKEN_SUBSTR_LEN 64
#define TOKEN_MAX_COUNT 256
#define TOKEN_MAX_COUNT_LOG 8
#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expression[TOKEN_SUBSTR_LEN];

  long long old_value;

  bool is_hex;

} WP;

enum {
  TK_CATAGORY_OTHER = 0,
  TK_CATAGORY_OPERATOR,
  TK_CATAGORY_OPERATOR_SINGLE,
  TK_CATAGORY_OPERAND
};

typedef struct token {
  int type;
  char str[TOKEN_SUBSTR_LEN];
  int str_sz;
  int catagry;
  int priority;
  int layer;
} Token;

long long expr(const char *e, bool *success);

WP *new_wp(const char *expression, long long value, bool is_hex);
void free_wp(WP *wp);
bool exam_watchers(void);
void list_watchers(void);

#endif
