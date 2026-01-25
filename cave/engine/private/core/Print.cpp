#include "cave/runtime/core/Print.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"

namespace cave {

void PrintImpl(LogLevel p_level, const std::string& p_message) {
    if (OS* os = OS::GetSingletonPtr()) [[likely]] {
        os->Print(p_level, p_message);
    } else {
        StdLogger logger;
        logger.Print(p_level, p_message);
    }
}

void LogImpl(LogLevel p_level, const std::string& p_message) {
    if (OS* os = OS::GetSingletonPtr()) [[likely]] {
        using namespace std::chrono;

        const uint32_t thread_id = thread::GetThreadId();
        std::string thread_info;

        if constexpr (false) {
            thread_info = std::format(" (thread: {})", thread_id);
        }

        auto now = floor<seconds>(system_clock::now());
        auto local = zoned_time{ current_zone(), now };
        auto message = std::format("[{:%H:%M:%S}]{} {}\n", local, thread_info, p_message);
        os->Print(p_level, message);
    } else {
        printf("%s\n", p_message.c_str());
    }
}

}  // namespace cave
