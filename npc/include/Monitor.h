#pragma once
#include <string>
enum class CmdResult { OKAY, INVALID_ARG, ERROR, QUIT };

CmdResult cmd_c(std::string arg);
CmdResult cmd_si(std::string arg);