// =============================================================================
// File: cave/framework/intent/IntentHandler.h
// =============================================================================
#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/runtime/intent/Intent.h"

namespace cave {

class IIntentHandler {
public:
    virtual ~IIntentHandler() = default;

    [[nodiscard]]
    virtual bool handleIntent(Intent& intent) = 0;

    virtual DebugId debugId() const = 0;
};

}  // namespace cave