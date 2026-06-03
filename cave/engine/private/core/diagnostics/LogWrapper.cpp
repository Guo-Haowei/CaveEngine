#include "cave/core/diagnostics/LogWrapper.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"

namespace cave {

void LogWrapper::Log(LogLevel p_level, LogChannel p_channel, std::string&& p_message) {
    LogEvent log = detail::BuildLog(p_level, p_channel, std::move(p_message));
    m_sink.Submit(std::move(log));
}

}  // namespace cave
