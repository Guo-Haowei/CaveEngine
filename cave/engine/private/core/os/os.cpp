#include "os.h"

namespace cave {

void OS::Finalize() {
    SetLogger(nullptr);
}

void OS::addLogger(std::unique_ptr<ILogSink>&& logger) {
    logger_.addLogger(std::move(logger));
}

}  // namespace cave