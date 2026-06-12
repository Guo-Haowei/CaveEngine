#include "cave/core/diagnostics/Log.h"

#include <chrono>

namespace cave {

namespace {

// @TODO: move to time util?
int64_t GetTimestampMs() {
    using namespace std::chrono;

    auto time = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(time).count();
}

// @TODO: move to time util?
void TimestampMsToHHMMSSmm(int64_t timestamp_ms,
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

LogEvent BuildLog(LogLevel level, LogChannel channel, std::string message) {
    LogEvent log;
    log.level = level;
    log.channel = channel;
    log.timestamp_ms = GetTimestampMs();
    TimestampMsToHHMMSSmm(log.timestamp_ms, log.time_str, sizeof(log.time_str));
    log.repeat = 1;
    log.message = std::move(message);
    return log;
}

}  // namespace

void LogImpl(LogLevel level, LogChannel channel, std::string message) {
    printf("%d %d %s\n", level, channel, message.c_str());

    // if (OS* os = OS::GetSingletonPtr()) [[likely]] {
    //     LogEvent log = detail::BuildLog(p_level, p_channel, std::move(p_message));
    //     os->Print(std::move(log));
    // } else {
    //     printf("%s\n", p_message.c_str());
    // }
}

}  // namespace cave
