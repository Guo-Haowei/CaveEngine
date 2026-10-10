// =============================================================================
// File: cave/core/diagnostics/CompositeLogger.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/CoreExport.h"
#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/memory/Pointer.h"

namespace cave {

class CAVE_CORE_API CompositeLogger : public ILogSink {
public:
    explicit CompositeLogger();
    ~CompositeLogger();

    void submit(const LogEvent& log) override;

    void addLogger(Owner<ILogSink>&& logger);

    void addLevel(LogLevel level);
    void removeLevel(LogLevel level);

    void flush();

    void clearLog();

    std::span<const LogEvent> allLogs() const;
    std::span<const LogEvent> warningLogs() const;
    std::span<const LogEvent> errorLogs() const;

private:
    class Impl;

    Impl* m_impl{};
};

}  // namespace cave
