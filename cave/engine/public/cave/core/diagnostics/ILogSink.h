// =============================================================================
// File: engine/public/cave/core/diagnostics/ILogSink.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/Log.h"

namespace cave {

class ILogSink {
public:
    virtual ~ILogSink() = default;

    virtual void Submit(const LogEvent& p_log) = 0;

    void Submit(LogLevel p_level, std::string p_message);
};

}  // namespace cave