#include "cave/core/diagnostics/Log.h"
#include "cave/core/diagnostics/ILogSink.h"

#include <chrono>
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

// @TODO: move to time util?
static int64_t GetTimestampMs() {
    using namespace std::chrono;

    auto time = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(time).count();
}

// @TODO: move to time util?
static void TimestampMsToHHMMSSmm(int64_t timestamp_ms,
                                  char* out,
                                  size_t out_size) {
    using namespace std::chrono;

    if (out_size < LogEvent::kMaxTimeString) {
        if (out_size > 0) {
            out[0] = '\0';
        }
        return;
    }

    auto tp = system_clock::time_point{ milliseconds{ timestamp_ms } };
    auto seconds_tp = time_point_cast<seconds>(tp);
    auto ms_part = duration_cast<milliseconds>(tp - seconds_tp).count();

    std::time_t t = system_clock::to_time_t(tp);

    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif

    std::snprintf(out,
                  out_size,
                  "%02d:%02d:%02d.%02lld",
                  local_tm.tm_hour,
                  local_tm.tm_min,
                  local_tm.tm_sec,
                  static_cast<long long>(ms_part));
}

static LogEvent BuildLog(LogLevel level, LogChannel channel, std::string message) {
    LogEvent log;
    log.level = level;
    log.channel = channel;
    log.timestamp_ms = GetTimestampMs();
    TimestampMsToHHMMSSmm(log.timestamp_ms, log.time_str, sizeof(log.time_str));
    log.repeat = 1;
    log.message = std::move(message);
    return log;
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
        LogEvent log = BuildLog(level, channel, std::move(message));
        s_log_glob.logger->Submit(log);
    } else {
        fprintf(stdout, "%s\n", message.c_str());
    }

    if (level & LOG_LEVEL_FATAL) {
        GENERATE_TRAP();
    }
}

}  // namespace cave
