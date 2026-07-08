// =============================================================================
// File: cave/core/diagnostics/LogWrapper.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class CAVE_CORE_API LogWrapper {
public:
    explicit LogWrapper(ILogSink& sink) noexcept
        : m_sink(sink) {}

    void Trace(LogChannel channel, std::string&& message) {
        Log(LOG_LEVEL_TRACE, channel, std::move(message));
    }

    void Info(LogChannel channel, std::string&& message) {
        Log(LOG_LEVEL_INFO, channel, std::move(message));
    }

    void Ok(LogChannel channel, std::string&& message) {
        Log(LOG_LEVEL_OK, channel, std::move(message));
    }

    void Warn(LogChannel channel, std::string&& message) {
        Log(LOG_LEVEL_WARN, channel, std::move(message));
    }

    void Error(LogChannel channel, std::string&& message) {
        Log(LOG_LEVEL_ERROR, channel, std::move(message));
    }

private:
    void Log(LogLevel level, LogChannel channel, std::string&& message);

    ILogSink& m_sink;
};

}  // namespace cave