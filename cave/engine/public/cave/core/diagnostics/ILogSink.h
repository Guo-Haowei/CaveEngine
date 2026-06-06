// =============================================================================
// File: cave/core/diagnostics/ILogSink.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/Log.h"

namespace cave {

class ILogSink {
public:
    virtual ~ILogSink() = default;

    virtual void Submit(const LogEvent& p_log) = 0;
};

}  // namespace cave