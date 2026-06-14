#pragma once
#include "cave/core/base/Singleton.h"

#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void Initialize();
    void Finalize();

    void AddLogger(std::shared_ptr<ILogSink> logger);

protected:
    CompositeLogger logger_;
};

bool IsAnsiSupported();

bool EnableAnsi();

}  // namespace cave