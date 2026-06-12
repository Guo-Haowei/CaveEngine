#include "os.h"

namespace cave {

void OS::Finalize() {
    SetLogger(nullptr);
}

void OS::AddLogger(std::shared_ptr<ILogSink> logger) {
    logger_.AddLogger(logger);
}

}  // namespace cave