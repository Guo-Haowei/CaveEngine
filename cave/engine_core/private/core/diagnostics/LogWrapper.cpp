#include "cave/core/diagnostics/LogWrapper.h"

// #include "engine/private/core/diagnostics/log_sink/LogUtils.h"

#include <cstdio>

namespace cave {

void LogWrapper::Log(LogLevel level, LogChannel channel, std::string&& message) {
    printf("%d %d %s\n", level, channel, message.c_str());
    // LogEvent log = detail::BuildLog(p_level, p_channel, std::move(p_message));
    // m_sink.Submit(std::move(log));
}

}  // namespace cave
