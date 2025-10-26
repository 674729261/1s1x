#pragma once
#include <cstdint>
#include <elfio/elfio.hpp>
#include <string>
#include <vector>

class ProgSymTab {
public:
  typedef struct {
    std::string name;
    uint32_t start;
  } SymbolItem;
  struct SymbolsTable {
    int symbol_count;
    std::vector<SymbolItem> symbol_items;
    std::vector<int> symbol_map;
  };
  typedef struct {
    int symbol;
    uint32_t pc;
  } Call;

  ProgSymTab(std::string_view elf_path) { init_and_parse(elf_path); }

  void init_and_parse(std::string_view elf_path);

  void push_call_stack(int symbol, uint32_t pc);
  Call pop_call_stack();
  int stack_cnt() { return call_stack.size(); }
  const std::string &find_symbol_name(int idx);
  int find_symbol_by_addr(uint32_t addr);

private:
  SymbolsTable table;
  std::vector<Call> call_stack;
  uint32_t pc_offset;
};