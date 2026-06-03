#pragma once
#include "engine/private/core/diagnostics/log_sink/Logger.h"

namespace cave {

class AnsiLogger : public ILogSink {
public:
    void Submit(const LogEvent& p_log) override;
};

}  // namespace cave
