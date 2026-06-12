#include "os.h"

#include "engine/private/core/io/file_access_unix.h"

namespace cave {

void OS::Finalize() {
    RemoveLogger(&logger_);
}

void OS::AddLogger(std::shared_ptr<ILogSink> logger) {
    logger_.AddLogger(logger);
}

void OS::Print(const LogEvent& p_log) {
    logger_.Submit(p_log);
    if (p_log.level & LOG_LEVEL_FATAL) {
        GENERATE_TRAP();
    }
}

}  // namespace cave