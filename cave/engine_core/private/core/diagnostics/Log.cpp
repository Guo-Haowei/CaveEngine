#include "cave/core/diagnostics/Log.h"
#include "cave/core/diagnostics/ILogSink.h"

#include "LogUtils.h"

#include <mutex>

namespace cave {

static struct {
    std::mutex guard;
    ILogSink* logger;
} s_log_glob;

void SetLogger(ILogSink* logger) {
    s_log_glob.guard.lock();
    s_log_glob.logger = logger;
    s_log_glob.guard.unlock();
}

std::string FormatLog(const LogEvent& log) {
    return std::format("[{}]  {}  {}  {}\n",
                       log.time_str,
                       ToString(log.level),
                       ToString(log.channel),
                       log.message);
}

void LogImpl(LogLevel level, LogChannel channel, std::string message) {
    if (s_log_glob.logger) {
        LogEvent log = detail::BuildLog(level, channel, std::move(message));
        s_log_glob.logger->submit(log);
    } else {
        fprintf(stdout, "%s\n", message.c_str());
    }

    if (level & LOG_LEVEL_FATAL) {
        GENERATE_TRAP();
    }
}

}  // namespace cave
