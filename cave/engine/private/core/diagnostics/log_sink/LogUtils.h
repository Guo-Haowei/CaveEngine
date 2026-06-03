#pragma once
#include "cave/core/diagnostics/Log.h"

namespace cave::detail {

LogEvent BuildLog(LogLevel p_level, std::string p_message);

std::string FormatLog(const LogEvent& p_log);

const char* ToString(LogLevel p_level);

}  // namespace cave::detail
