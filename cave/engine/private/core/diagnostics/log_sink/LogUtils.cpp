#include "LogUtils.h"

namespace cave {

// @TODO: refactor
static int64_t GetTimestampMs() {
    using namespace std::chrono;

    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch())
        .count();
}

// @TODO: refactor
inline void TimestampMsToHHMMSSmm(int64_t timestamp_ms,
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
}  // namespace cave

namespace cave::detail {

LogEvent BuildLog(LogLevel p_level, LogChannel p_channel, std::string p_message) {
    LogEvent log;
    log.level = p_level;
    log.channel = p_channel;
    log.timestamp_ms = GetTimestampMs();
    TimestampMsToHHMMSSmm(log.timestamp_ms, log.time_str, sizeof(log.time_str));
    log.repeat = 1;
    log.message = std::move(p_message);
    return log;
}

std::string FormatLog(const LogEvent& p_log) {
    auto log = std::format("{}  {}  {}  {}\n",
                           p_log.time_str,
                           detail::ToString(p_log.level),
                           detail::ToString(p_log.channel),
                           p_log.message);

    return log;
}

const char* ToString(LogLevel p_level) {
    switch (p_level) {
#define LOG_LEVEL_COLOR(LEVEL, TAG, ANSI, WINCOLOR) \
    case LEVEL:                                     \
        return TAG;
        LOG_LEVEL_COLOR_LIST
#undef LOG_LEVEL_COLOR
        default:
            return "";
    }
}

const char* ToString(LogChannel p_channel) {
    DEV_ASSERT_INDEX(p_channel, LogChannel::Count);
    static constexpr const char* s_channels[] = {
#define CAVE_LOG_CHANNEL(ENUM, STR) STR,
        CAVE_LOG_CHANNEL_LIST
#undef CAVE_LOG_CHANNEL
    };
    static_assert(std::size(s_channels) == std::to_underlying(LogChannel::Count));
    return s_channels[std::to_underlying(p_channel)];
}

}  // namespace cave::detail
