#pragma once
#include <cstdint>

#define CONFIG_MSIZE 0x8000000

typedef struct {
  char *name;
  uint32_t start;
} SymbolItem;

struct SymbolsTable {
  int symbol_count;
  SymbolItem *symbol_items;
  int symbol_map[CONFIG_MSIZE];
};
extern struct SymbolsTable symbols_table;

long load_symbols(const char *elf);
int find_symbol(uint32_t addr);
const char *find_symbol_name(int idx);

typedef struct {
  int symbol;
  uint32_t pc;
} Call;

extern Call *stack_ftrace;
extern int cnt_stack_ftrace;

void push_stack_ftrace(uint32_t pc, int symbol);
Call pop_stack_ftrace(void);
