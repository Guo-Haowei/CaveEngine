#include "os.h"

namespace cave {

void OS::Finalize() {
    SetLogger(nullptr);
}

void OS::addLogger(Owner<ILogSink>&& logger) {
    m_logger.addLogger(std::move(logger));
}

}  // namespace cave
