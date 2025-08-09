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

WP *new_wp(const char *expression) {
  if (free_ == NULL) {
    printf("No more free watchers.\n");
    return NULL;
  }
  WP *ret = free_;
  free_ = free_->next;
  ret->next = head;
  head = ret;
  strncpy(ret->expression, expression, TOKEN_SUBSTR_LEN);
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
