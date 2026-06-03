#include "cave/core/diagnostics/Log.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"
#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"

namespace cave {

void LogImpl(LogLevel p_level, std::string p_message) {
    LogEvent log = detail::BuildLog(p_level, std::move(p_message));

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
