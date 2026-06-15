// =============================================================================
// File: cave/core/diagnostics/ILogSink.h
// =============================================================================
#pragma once
#include "cave/core/CoreExport.h"
#include "cave/core/diagnostics/Log.h"

namespace cave {

class CAVE_CORE_API ILogSink {
public:
    virtual ~ILogSink() = default;

    virtual void submit(const LogEvent& log) = 0;
};

}  // namespace cave