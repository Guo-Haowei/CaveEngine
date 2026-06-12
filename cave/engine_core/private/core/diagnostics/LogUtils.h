#pragma once
#include "cave/core/diagnostics/Log.h"

namespace cave::detail {

LogEvent BuildLog(LogLevel level, LogChannel channel, std::string message);

}  // namespace cave::detail
