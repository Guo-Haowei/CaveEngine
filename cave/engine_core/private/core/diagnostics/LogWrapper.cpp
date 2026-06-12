#include "cave/core/diagnostics/LogWrapper.h"

#include "LogUtils.h"

#include <cstdio>

namespace cave {

void LogWrapper::Log(LogLevel level, LogChannel channel, std::string&& message) {
    LogEvent log = detail::BuildLog(level, channel, std::move(message));
    sink_.Submit(std::move(log));
}

}  // namespace cave
