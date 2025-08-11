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
#include "debug.h"
#include "sdb.h"
#include <stdio.h>
#include <string.h>

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP *new_wp(const char *expression, long long value, bool is_hex) {
  if (free_ == NULL)
    return NULL;
  WP *ret = free_;
  free_ = free_->next;
  ret->next = head;
  head = ret;
  strncpy(ret->expression, expression, TOKEN_SUBSTR_LEN);
  ret->old_value = value;
  ret->is_hex = is_hex;
  return ret;
}
void free_wp(WP *wp) {
  if (head == wp)
    head = wp->next;
  else {
    for (WP *iter = head; iter != NULL; iter = iter->next) {
      if (iter->next == wp) {
        iter->next = wp->next;
        break;
      }
    }
  }
  wp->next = free_;
  free_ = wp;
}

bool exam_watchers(void) {
  bool ret = false;
  for (WP *iter = head; iter != NULL; iter = iter->next) {
    bool success = true;
    long long now_value = expr(iter->expression, &success);
    Assert(success, "Expression evaluation failed while examing watcher #%d",
           iter->NO);
    if (now_value != iter->old_value) {
      printf("Watcher changed : #%d = %s \nfrom : %lld\nto   : %lld\n",
             iter->NO, iter->expression, iter->old_value, now_value);
      iter->old_value = now_value;
      ret = true;
    }
  }
  return ret;
}

void list_watchers(void) {
  int cnt_used = 0;
  if (head != NULL) {
    puts("---------------------------------------------------");
    puts("ID  |               value|                     expr");
    puts("---------------------------------------------------");
    for (WP *iter = head; iter != NULL; iter = iter->next) {
      ++cnt_used;
      if (iter->is_hex)
        printf("%4d|          0x%08x|%25s\n", iter->NO, (word_t)iter->old_value,
               iter->expression);
      else
        printf("%4d|%20lld|%25s\n", iter->NO, iter->old_value,
               iter->expression);
    }
    puts("---------------------------------------------------");
  }
  printf("%d active watchers, %d free watchers.\n", cnt_used, NR_WP - cnt_used);
}
