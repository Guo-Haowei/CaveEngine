#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class AnsiLogger : public ILogSink {
public:
    void submit(const LogEvent& p_log) override;
};

}  // namespace cave
