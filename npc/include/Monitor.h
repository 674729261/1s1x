#pragma once
#include <string_view>
enum class CmdResult { OKAY, INVALID_ARG, ERROR, QUIT };

CmdResult cmd_c(std::string_view arg);
CmdResult cmd_si(std::string_view arg);
CmdResult cmd_x(std::string_view arg);
CmdResult cmd_q(std::string_view arg);

using Cmd_Func = CmdResult (*)(std::string_view);

const struct {
  Cmd_Func call;
  std::string_view command;
  std::string_view help;
} cmd_list[] = {
    {cmd_c, "c", "Start running. Useage : c"},
    {cmd_si, "si", "Step specific instructions. Useage : si [# of steps]"},
    {cmd_si, "x", "Scan memory. Useage : x (# of words) (base address)"},
    {cmd_q, "q", "Quit"}};
