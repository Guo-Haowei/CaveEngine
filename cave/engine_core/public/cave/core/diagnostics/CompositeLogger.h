// =============================================================================
// File: cave/core/diagnostics/CompositeLogger.h
// =============================================================================
#pragma once
#include <memory>
#include <span>

#include "cave/core/CoreExport.h"
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class CAVE_CORE_API CompositeLogger : public ILogSink {
public:
    explicit CompositeLogger();
    ~CompositeLogger();

    void submit(const LogEvent& log) override;

    void addLogger(std::unique_ptr<ILogSink>&& logger);

    void addLevel(LogLevel level);
    void removeLevel(LogLevel level);

    void flush();

    void clearLog();

    std::span<const LogEvent> allLogs() const;
    std::span<const LogEvent> warningLogs() const;
    std::span<const LogEvent> errorLogs() const;

    static CompositeLogger& singleton();

private:
    class Impl;

    Impl* impl_{};
};

}  // namespace cave
