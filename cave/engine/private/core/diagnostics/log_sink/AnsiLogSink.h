#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class AnsiLogger : public ILogSink {
public:
    void Submit(const LogEvent& p_log) override;
};

}  // namespace cave
