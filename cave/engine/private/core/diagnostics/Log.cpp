#include "cave/core/diagnostics/Log.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"

namespace cave {

enum class LogChannel : uint16_t {
    Default,
};

// @TODO: refactor
static inline int64_t GetTimestampMs() {
    using namespace std::chrono;

    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch())
        .count();
}

Log BuildLog(LogLevel p_level, std::string p_message) {
    return {
        .level = p_level,
        .channel = LogChannel::Default,
        .repeat = 1,
        .timestamp_ms = GetTimestampMs(),
        .message = std::move(p_message),
    };
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
