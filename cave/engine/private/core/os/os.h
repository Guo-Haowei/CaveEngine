#pragma once
#include "cave/core/Singleton.h"

#include "engine/private/core/diagnostics/log_sink/Logger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void Initialize();
    void Finalize();

    virtual void Print(const LogEvent& p_log);

    void AddLogger(std::shared_ptr<ILogSink> p_logger);

protected:
    CompositeLogger m_logger;
};

bool IsAnsiSupported();

bool EnableAnsi();

}  // namespace cave