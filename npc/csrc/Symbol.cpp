#include "Symbols.h"
#include <cstdint>
#include <elf.h>
#include <format>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

const uint32_t CONFIG_MBASE = 0x80000000;

void Assert(bool cond, const std::string &msg) {
  if (!cond)
    throw std::runtime_error(msg);
}

struct SymbolsTable symbols_table = {};
static int allocated_sz_stack = 0;

static void parse_symbols(const Elf32_Ehdr *elf_header) {
  Elf32_Shdr *sections =
      (Elf32_Shdr *)((char *)elf_header + elf_header->e_shoff);
  int allocated_size = 4;
  symbols_table.symbol_items = (SymbolItem *)malloc(sizeof(SymbolItem) * 4);
  symbols_table.symbol_count = 0;
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (sections[i].sh_type == SHT_SYMTAB) {
      Elf32_Shdr *symtab = &sections[i];
      Elf32_Sym *symbols =
          (Elf32_Sym *)((char *)elf_header + symtab->sh_offset);
      int count = symtab->sh_size / symtab->sh_entsize;
      const char *symstrtab =
          (char *)elf_header + sections[symtab->sh_link].sh_offset;

      for (int i = 0; i < count; i++) {
        if (ELF32_ST_TYPE(symbols[i].st_info) != STT_FUNC)
          continue;

        fprintf(stderr, "%3d : %08x %4d %s\n", symbols_table.symbol_count,
                symbols[i].st_value, symbols[i].st_size,
                &symstrtab[symbols[i].st_name]);
        int name_len = strlen(&symstrtab[symbols[i].st_name]);
        if (allocated_size == symbols_table.symbol_count) {
          symbols_table.symbol_items =
              (SymbolItem *)realloc(symbols_table.symbol_items,
                                    sizeof(SymbolItem) * allocated_size * 2);
          Assert(symbols_table.symbol_items,
                 "Failed to allocate memory for symbols");
          allocated_size *= 2;
        }
        symbols_table.symbol_items[symbols_table.symbol_count].name =
            (char *)malloc(name_len + 1);
        Assert(symbols_table.symbol_items[symbols_table.symbol_count].name,
               "Failed to allocate memory for symbol name");
        strncpy(symbols_table.symbol_items[symbols_table.symbol_count].name,
                &symstrtab[symbols[i].st_name], name_len);
        symbols_table.symbol_items[symbols_table.symbol_count].name[name_len] =
            '\0';
        symbols_table.symbol_items[symbols_table.symbol_count].start =
            symbols[i].st_value;
        for (uint32_t addr = symbols[i].st_value;
             addr < symbols[i].st_value + symbols[i].st_size; addr++) {
          symbols_table.symbol_map[addr - CONFIG_MBASE] =
              symbols_table.symbol_count;
        }
        symbols_table.symbol_count++;
      }
    }
  }
}

long load_symbols(const char *elf) {
  FILE *fp = fopen(elf, "rb");
  Assert(fp, std::format("Failed to load elf : {}.", elf));
  fseek(fp, 0, SEEK_END);
  size_t size_file = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *elf_data = (char *)malloc(size_file);
  fread(elf_data, size_file, 1, fp);
  fclose(fp);
  Elf32_Ehdr *elf_header = (Elf32_Ehdr *)elf_data;

  //   parse_symbols(elf_header);
  free(elf_data);
  stack_ftrace = (Call *)malloc(sizeof(Call) * 4);
  allocated_sz_stack = 4;
  return symbols_table.symbol_count;
}
int find_symbol(uint32_t addr) {
  return symbols_table.symbol_map[addr - CONFIG_MBASE];
}
const char *find_symbol_name(int idx) {
  Assert(idx >= 0 && idx < symbols_table.symbol_count,
         std::format("Invalid symbol id {}", idx));
  return symbols_table.symbol_items[idx].name;
}

void free_symbols() {
  for (int i = 0; i < symbols_table.symbol_count; i++)
    if (symbols_table.symbol_items[i].name)
      free(symbols_table.symbol_items[i].name);
  if (symbols_table.symbol_items)
    free(symbols_table.symbol_items);
  if (stack_ftrace)
    free(stack_ftrace);
}

Call *stack_ftrace;

int cnt_stack_ftrace = 0;

void push_stack_ftrace(uint32_t pc, int symbol) {
  if (cnt_stack_ftrace == allocated_sz_stack) {
    stack_ftrace =
        (Call *)realloc(stack_ftrace, sizeof(Call) * allocated_sz_stack * 2);
    Assert(stack_ftrace, "Failed to reallocate memory for stack ftrace");
    allocated_sz_stack *= 2;
  }
  stack_ftrace[cnt_stack_ftrace].symbol = symbol;
  stack_ftrace[cnt_stack_ftrace].pc = pc;
  cnt_stack_ftrace++;
}
Call pop_stack_ftrace(void) {
  Assert(cnt_stack_ftrace >= 0, "Stack FTrace is empty.");
  cnt_stack_ftrace--;
  return stack_ftrace[cnt_stack_ftrace];
}