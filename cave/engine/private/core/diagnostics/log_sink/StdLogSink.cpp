#include "StdLogSink.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"

namespace cave {

void StdLogger::Submit(const LogEvent& p_log) {
    const char* tag = detail::ToString(p_log.level);

    // @TODO: stderr vs stdout
    FILE* file = stdout;
    fflush(file);

    fprintf(file, "%s%s", tag, p_log.message.c_str());
    fflush(file);
}

}  // namespace cave
