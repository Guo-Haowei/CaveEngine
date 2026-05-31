// =============================================================================
// File: engine/public/cave/framework/intent/IntentHandler.h
// =============================================================================
#pragma once
#include "Intent.h"

namespace cave {

class IIntentHandler {
public:
    virtual ~IIntentHandler() = default;

    virtual void HandleIntent(const Intent& p_intent) = 0;
};

}  // namespace cave