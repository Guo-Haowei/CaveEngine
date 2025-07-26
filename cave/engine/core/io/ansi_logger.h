#pragma once
#include "engine/core/io/logger.h"

namespace cave {

class AnsiLogger : public ILogger {
public:
    void Print(LogLevel p_level, std::string_view p_message) override;
};

}  // namespace cave
