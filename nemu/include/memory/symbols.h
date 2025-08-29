#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__
#include "common.h"
struct SymbolsTable {
  int symbol_count;
  int memory_count;
  char **symbol_strings;
  int *symbol_map;
};
extern struct SymbolsTable symbols_table;

long load_symbols(char *elf);
int find_symbol(vaddr_t addr);
const char *find_symbol_name(int idx);
#endif