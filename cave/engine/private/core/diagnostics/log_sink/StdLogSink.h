#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class StdLogger : public ILogSink {
public:
    virtual void Submit(const LogEvent& p_log) override;
};

}  // namespace cave
