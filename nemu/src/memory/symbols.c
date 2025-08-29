#include "memory/symbols.h"
#include "common.h"
#include "debug.h"
#include "memory/paddr.h"
#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct SymbolsTable symbols_table = {};

static int parse_symbols(const Elf32_Ehdr *elf_header) {
  Elf32_Shdr *sections =
      (Elf32_Shdr *)((char *)elf_header + elf_header->e_shoff);
  int cnt_func = 0;
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (sections[i].sh_type == SHT_SYMTAB) {
      Elf32_Shdr *symtab = &sections[i];
      Elf32_Sym *symbols =
          (Elf32_Sym *)((char *)elf_header + symtab->sh_offset);
      int count = symtab->sh_size / symtab->sh_entsize;
      for (int i = 0; i < count; i++) {
        if (ELF32_ST_TYPE(symbols[i].st_info) == STT_FUNC)
          cnt_func++;
      }
    }
  }
  symbols_table.symbol_strings = malloc(sizeof(char *) * cnt_func);
  memset(symbols_table.symbol_strings, 0, sizeof(char *) * cnt_func);
  cnt_func = 0;
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

        fprintf(stderr, "%08x %d %s\n", symbols[i].st_value, symbols[i].st_size,
                &symstrtab[symbols[i].st_name]);
        int name_len = strlen(&symstrtab[symbols[i].st_name]);
        symbols_table.symbol_strings[cnt_func] = malloc(name_len + 1);
        Assert(symbols_table.symbol_strings[cnt_func],
               "Failed to allocate memory for symbol name");
        strncpy(symbols_table.symbol_strings[cnt_func],
                &symstrtab[symbols[i].st_name], name_len);
        symbols_table.symbol_strings[cnt_func][name_len] = '\0';
        for (vaddr_t addr = symbols[i].st_value;
             addr < symbols[i].st_value + symbols[i].st_size; addr++) {
          Assert(in_pmem(addr), "Invalid symbol addr 0x%08x\n", addr);
          symbols_table.symbol_map[addr - CONFIG_MBASE] = cnt_func;
        }
        cnt_func++;
      }
    }
  }
  return cnt_func;
}

long load_symbols(char *elf) {
  FILE *fp = fopen(elf, "rb");
  Assert(fp, "Failed to load elf : %s.", elf);
  fseek(fp, 0, SEEK_END);
  size_t size_file = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *elf_data = malloc(size_file);
  fread(elf_data, size_file, 1, fp);
  fclose(fp);
  Elf32_Ehdr *elf_header = (Elf32_Ehdr *)elf_data;

  int num_funcs = parse_symbols(elf_header);
  free(elf_data);
  symbols_table.symbol_count = num_funcs;
  return num_funcs;
}
int find_symbol(vaddr_t addr) {
  Assert(in_pmem(addr), "Invalid addr 0x%08x\n", addr);
  return symbols_table.symbol_map[addr - CONFIG_MBASE];
}
const char *find_symbol_name(int idx) {
  Assert(idx >= 0 && idx < symbols_table.symbol_count, "Invalid symbol id %d\n",
         idx);
  return symbols_table.symbol_strings[idx];
}

void free_symbols() {
  for (int i = 0; i < symbols_table.symbol_count; i++)
    if (symbols_table.symbol_strings[i])
      free(symbols_table.symbol_strings[i]);
}

Call stack_ftrace[MAX_STACK_FTRACE];
int cnt_stack_ftrace = 0;

void push_stack_ftrace(vaddr_t pc, int symbol) {
  Assert(cnt_stack_ftrace < MAX_STACK_FTRACE, "Stack FTrace is full.");
  stack_ftrace[cnt_stack_ftrace].symbol = symbol;
  stack_ftrace[cnt_stack_ftrace].pc = pc;
  cnt_stack_ftrace++;
}
Call pop_stack_ftrace(void) {
  Assert(cnt_stack_ftrace >= 0, "Stack FTrace is empty.");
  cnt_stack_ftrace--;
  return stack_ftrace[cnt_stack_ftrace];
}