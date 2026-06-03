#include "cave/core/diagnostics/Log.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"
#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"

namespace cave {

void LogImpl(LogLevel p_level, LogChannel p_channel, std::string p_message) {
    if (OS* os = OS::GetSingletonPtr()) [[likely]] {
        LogEvent log = detail::BuildLog(p_level, p_channel, std::move(p_message));
        os->Print(std::move(log));
    } else {
        printf("%s\n", p_message.c_str());
    }
}

}  // namespace cave
