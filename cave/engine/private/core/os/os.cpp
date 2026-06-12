#include "os.h"

#include "engine/private/core/io/file_access_unix.h"

namespace cave {

void OS::Finalize() {
    RemoveLogger(&logger_);
}

void OS::AddLogger(std::shared_ptr<ILogSink> logger) {
    logger_.AddLogger(logger);
}

}  // namespace cave