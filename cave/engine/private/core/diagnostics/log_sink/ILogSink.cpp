#include "cave/core/diagnostics/ILogSink.h"

#include "LogUtils.h"

namespace cave {

void ILogSink::Submit(LogLevel p_level, std::string p_message) {
    LogEvent log = detail::BuildLog(p_level, std::move(p_message));
    Submit(log);
}

}  // namespace cave
