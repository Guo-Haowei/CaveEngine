#pragma once
#include "cave/core/base/Singleton.h"
#include "cave/core/diagnostics/CompositeLogger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void Initialize();
    void Finalize();

    void addLogger(Owner<ILogSink>&& logger);

    CompositeLogger& logger() { return m_logger; }

protected:
    CompositeLogger m_logger;
};

bool IsAnsiSupported();

bool EnableAnsi();

}  // namespace cave
