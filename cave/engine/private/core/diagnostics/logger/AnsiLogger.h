#pragma once
#include "engine/private/core/diagnostics/logger/Logger.h"

namespace cave {

class AnsiLogger : public ILogger {
public:
    void Print(const Log& p_log) override;
};

}  // namespace cave
