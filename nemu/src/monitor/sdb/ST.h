#ifndef __ST_H__
#define __ST_H__
#include "debug.h"
#include "sdb.h"

typedef struct {
  const Token *ref;
  int table[TOKEN_MAX_COUNT_LOG][TOKEN_MAX_COUNT];
  int n;
} TokenST;

extern int ST_Log2[TOKEN_MAX_COUNT];

void pre_log2();

void init_ST(TokenST *st, int n, const Token *ref);
int query_ST(TokenST *st, int from, int to);

#endif