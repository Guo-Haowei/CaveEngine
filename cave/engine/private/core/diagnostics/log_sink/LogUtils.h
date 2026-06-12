#pragma once
#include "cave/core/diagnostics/Log.h"

// WORD is flags of FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
// clang-format off
//                  Level,              TAG       Ansi            DWORD
#define LOG_LEVEL_COLOR_LIST                                           \
    LOG_LEVEL_COLOR(LOG_LEVEL_TRACE,    "TRACE",  "\033[90m",     0x8) \
    LOG_LEVEL_COLOR(LOG_LEVEL_INFO,     "INFO ",  "\033[0m",      0x7) \
    LOG_LEVEL_COLOR(LOG_LEVEL_OK,       "OK   ",  "\033[92m",     0xA) \
    LOG_LEVEL_COLOR(LOG_LEVEL_WARN,     "WARN ",  "\033[93m",     0xE) \
    LOG_LEVEL_COLOR(LOG_LEVEL_ERROR,    "ERROR",  "\033[91m",     0xC) \
    LOG_LEVEL_COLOR(LOG_LEVEL_FATAL,    "FATAL",  "\033[101;30m", 0xC)
// clang-format on

namespace cave::detail {

LogEvent BuildLog(LogLevel p_level, LogChannel p_channel, std::string p_message);

}  // namespace cave::detail
