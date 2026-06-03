#include "cave/core/diagnostics/ILogSink.h"

#include "LogUtils.h"

namespace cave {

void ILogSink::Submit(LogLevel p_level, std::string p_message, LogChannel p_channel) {
    LogEvent log = detail::BuildLog(p_level, p_channel, std::move(p_message));
    Submit(log);
}

}  // namespace cave
