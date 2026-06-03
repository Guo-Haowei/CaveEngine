#include "Win32DebuggerSink.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

void DebugConsoleLogger::Submit(const LogEvent& p_log) {
    auto log = detail::FormatLog(p_log);

    OutputDebugStringA(log.c_str());
}

}  // namespace cave
