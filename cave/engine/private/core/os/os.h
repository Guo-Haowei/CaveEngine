#pragma once
#include "cave/core/base/Singleton.h"
#include "cave/core/diagnostics/CompositeLogger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void Initialize();
    void Finalize();

    void addLogger(std::unique_ptr<ILogSink>&& logger);

protected:
    CompositeLogger logger_;
};

bool IsAnsiSupported();

bool EnableAnsi();

}  // namespace cave