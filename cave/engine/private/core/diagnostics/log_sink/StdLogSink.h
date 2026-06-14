#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class StdLogger : public ILogSink {
public:
    void submit(const LogEvent& log) override;
};

}  // namespace cave
