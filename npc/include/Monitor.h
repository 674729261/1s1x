#pragma once
#include <atomic>
#include <string_view>
enum class CmdResult { OKAY, INVALID_ARG, ERROR };

inline std::atomic_bool quit;

CmdResult cmd_c(std::string_view arg);
CmdResult cmd_si(std::string_view arg);
CmdResult cmd_x(std::string_view arg);
CmdResult cmd_q(std::string_view arg);
CmdResult cmd_li(std::string_view arg);

using Cmd_Func = CmdResult (*)(std::string_view);

const struct {
  Cmd_Func call;
  std::string_view command;
  std::string_view help;
} cmd_list[] = {
    {cmd_c, "c", "Start running. Usage : c"},
    {cmd_si, "si", "Step specific instructions. Usage : si [# of steps]"},
    {cmd_si, "x", "Scan memory. Usage : x (# of words) (base address)"},
    {cmd_q, "q", "Quit"},
    {cmd_li, "li", "List infomation. Usage : li (r|w)"}};
