#include "os.h"

#include "engine/private/core/io/file_access_unix.h"

namespace cave {

void OS::Finalize() {
}

void OS::AddLogger(std::shared_ptr<ILogSink> p_logger) {
    m_logger.AddLogger(p_logger);
}

void OS::Print(const LogEvent& p_log) {
    m_logger.Submit(p_log);
    if (p_log.level & LOG_LEVEL_FATAL) {
        GENERATE_TRAP();
    }
}

}  // namespace cave