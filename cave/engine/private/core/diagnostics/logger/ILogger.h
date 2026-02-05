// =============================================================================
// File: engine/public/cave/core/diagnostics/ILogger.h
// =============================================================================
#pragma once
#include "cave/core/Print.h"

namespace cave {

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void Print(LogLevel p_level, std::string_view p_message) = 0;
};

}  // namespace cave