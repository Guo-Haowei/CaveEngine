#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class StdLogger : public ILogSink {
public:
    void Submit(const LogEvent& log) override;
};

}  // namespace cave
