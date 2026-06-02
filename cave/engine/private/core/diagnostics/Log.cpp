#include "cave/core/diagnostics/Log.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"

namespace cave {

enum class LogChannel : uint16_t {
    Default = 0,
};

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

    if (out_size < Log::kMaxTimeString) {
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

Log BuildLog(LogLevel p_level, std::string p_message) {
    Log log;
    log.level = p_level;
    log.channel = LogChannel::Default;
    log.timestamp_ms = GetTimestampMs();
    TimestampMsToHHMMSSmm(log.timestamp_ms, log.time_str, sizeof(log.time_str));
    log.repeat = 1;
    log.message = std::move(p_message);
    return log;
}

void LogImpl(LogLevel p_level, std::string p_message) {
    Log log = BuildLog(p_level, std::move(p_message));

    if (OS* os = OS::GetSingletonPtr()) [[likely]] {
        // using namespace std::chrono;

        // auto now = floor<seconds>(system_clock::now());
        // auto local = zoned_time{ current_zone(), now };
        // auto message = std::format("[{:%H:%M:%S}] {}\n", local, p_message);
        os->Print(log);
    } else {
        printf("%s\n", log.message.c_str());
    }
}

}  // namespace cave
