#pragma once

#include <cctype>
#include <charconv>
#include <optional>
#include <ostream>
#include <print>
#include <string_view>
using std::optional;
using std::println, std::print;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

template <class T> inline std::optional<T> to_number(std::string_view p) {
  int base = 10, offset = 0;
  if (p.size() >= 2 && p[0] == '0' && std::tolower(p[1]) == 'x') {
    base = 16;
    offset = 2;
  } else if (p.size() > 1 && p[0] == '0') {
    base = 8;
    offset = 1;
  }
  T ret = -1;
  auto [ptr, ec] = std::from_chars(p.begin() + offset, p.end(), ret, base);
  if (ec == std::errc::result_out_of_range) {
    println("Argument is too large : {}", p);
    return std::nullopt;
  } else if (ptr != p.end() || ec != std::errc()) {
    println("Invalid argument : {}", p);
    return std::nullopt;
  }
  return ret;
}