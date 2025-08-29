#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__
#include "common.h"
#define MAX_STACK_FTRACE 128
struct SymbolsTable {
  int symbol_count;
  char **symbol_strings;
  int symbol_map[CONFIG_MSIZE];
};
extern struct SymbolsTable symbols_table;

long load_symbols(char *elf);
int find_symbol(vaddr_t addr);
const char *find_symbol_name(int idx);

typedef struct {
  int symbol;
  vaddr_t pc;
} Call;

extern Call stack_ftrace[];
extern int cnt_stack_ftrace;

void push_stack_ftrace(vaddr_t pc, int symbol);
Call pop_stack_ftrace(void);

#endif