#include "StdLogSink.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"

namespace cave {

void StdLogger::Submit(const LogEvent& log) {
    const char* tag = ToString(log.level);

    // @TODO: stderr vs stdout
    FILE* file = stdout;
    fflush(file);

    fprintf(file, "%s%s", tag, log.message.c_str());
    fflush(file);
}

}  // namespace cave
