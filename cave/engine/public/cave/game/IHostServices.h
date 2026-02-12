// =============================================================================
// File: engine/public/cave/game/IHostServices.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/ILogger.h"

namespace cave {

class IHostServices {
public:
    virtual ~IHostServices() = default;

    virtual ILogger& Log() = 0;
};

}  // namespace cave