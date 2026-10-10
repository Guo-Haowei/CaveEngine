#include "StdLogSink.h"

namespace cave {

void StdLogger::submit(const LogEvent& log) {
    // @TODO: stderr vs stdout
    FILE* file = stdout;
    fflush(file);

    fprintf(file, "[%s]  %s  %s  %s\n",
            log.time_str,
            ToString(log.level),
            ToString(log.channel),
            log.message.c_str());

    fflush(file);
}

}  // namespace cave
