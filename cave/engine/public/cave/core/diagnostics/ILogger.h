// =============================================================================
// File: engine/public/cave/core/diagnostics/ILogger.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/Log.h"

namespace cave {

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void Print(const LogEvent& p_log) = 0;

    void Print(LogLevel p_level, std::string p_message);
};

}  // namespace cave