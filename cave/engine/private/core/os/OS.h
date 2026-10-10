#pragma once
#include "cave/core/base/Singleton.h"
#include "cave/core/diagnostics/CompositeLogger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void initialize();
    void finalize();

    void addLogger(Owner<ILogSink>&& logger) {
        m_logger.addLogger(std::move(logger));
    }

    CompositeLogger& logger() { return m_logger; }

protected:
    CompositeLogger m_logger;
};

}  // namespace cave
