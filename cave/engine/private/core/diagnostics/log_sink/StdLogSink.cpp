#include "StdLogSink.h"

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
