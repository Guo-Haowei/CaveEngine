#pragma once
#include "cave/core/Singleton.h"

#include "engine/private/core/diagnostics/logger/Logger.h"

namespace cave {

class OS : public Singleton<OS> {
public:
    void Initialize();
    void Finalize();

    virtual void Print(LogLevel p_level, std::string_view p_message);

    void AddLogger(std::shared_ptr<ILogger> p_logger);

protected:
    CompositeLogger m_logger;
};

bool IsAnsiSupported();

bool EnableAnsi();

}  // namespace cave