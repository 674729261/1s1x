#pragma once

#include <charconv>
#include <optional>
#include <ostream>
#include <print>
#include <string_view>
using std::optional;
using std::println, std::print;

template <class T>
inline std::optional<T> to_number(std::string_view p, int base = 10) {
  T ret = -1;
  auto [ptr, ec] = std::from_chars(p.begin(), p.end(), ret, base);
  if (ec == std::errc::result_out_of_range) {
    println("Argument is too large : {}", p);
    return std::nullopt;
  } else if (ptr != p.end() || ec != std::errc()) {
    println("Invalid argument : {}", p);
    return std::nullopt;
  }
  return ret;
}