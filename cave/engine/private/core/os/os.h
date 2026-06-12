#pragma once
#include "cave/core/Singleton.h"

#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void Initialize();
    void Finalize();

    virtual void Print(const LogEvent& log);

    void AddLogger(std::shared_ptr<ILogSink> logger);

protected:
    CompositeLogger logger_;
};

bool IsAnsiSupported();

bool EnableAnsi();

}  // namespace cave