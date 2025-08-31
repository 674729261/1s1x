#include "Monitor/SingleMonitor.h"
#include <iostream>
#include <ostream>
#include <print>
#include <regex>
#include <string>
#include <utility>
#include <vector>

using std::cin, std::getline;
using std::print, std::println;
using std::regex, std::sregex_token_iterator;
using std::string;
using std::vector;
SingleMonitor::SingleMonitor(std::unique_ptr<RISCV32> dut,
                             unsigned int max_cycles, bool batch)
    : dut(std::move(dut)), max_cycles(max_cycles), batch(batch) {}

void SingleMonitor::start() {}

void SingleMonitor::query_command() {
  print("(NPCemu)");
  string command;
  getline(cin, command);
  regex del(R"(\s+)");

  sregex_token_iterator it(command.begin(), command.end(), del, -1);
  sregex_token_iterator end;
  vector<string> params;
  string head;
  while (it != end) {
    if (head.empty())
      head = *it;
    else
      params.push_back(*it);
  }
  if (!head.empty()) {
  }
}

int SingleMonitor::help() { return 0; }
int SingleMonitor::step() { return 0; }
int SingleMonitor::run() { return 0; }