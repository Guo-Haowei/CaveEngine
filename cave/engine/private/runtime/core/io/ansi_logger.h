#pragma once
#include "engine/private/runtime/core/io/logger.h"

namespace cave {

class AnsiLogger : public ILogger {
public:
    void Print(LogLevel p_level, std::string_view p_message) override;
};

}  // namespace cave
