#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__
#include "common.h"
#define SZ_SYMBOL_MAP 4096

struct SymbolsTable {
  int symbol_count;
  char *symbol_strings[SZ_SYMBOL_MAP];
  int symbol_map[SZ_SYMBOL_MAP];
};
extern struct SymbolsTable symbols_table;

long load_symbols(char *elf);
int find_symbol(vaddr_t addr);
const char *find_symbol_name(int idx);
#endif