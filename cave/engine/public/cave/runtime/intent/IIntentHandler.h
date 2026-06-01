// =============================================================================
// File: engine/public/cave/framework/intent/IntentHandler.h
// =============================================================================
#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/runtime/intent/Intent.h"

namespace cave {

class IIntentHandler {
public:
    virtual ~IIntentHandler() = default;

    virtual void HandleIntent(Intent& p_intent) = 0;

    virtual DebugId GetDebugId() const = 0;
};

}  // namespace cave