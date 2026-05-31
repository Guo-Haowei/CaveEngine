// =============================================================================
// File: engine/public/cave/framework/intent/IntentHandler.h
// =============================================================================
#pragma once
#include "Intent.h"

namespace cave {

enum class IntentResult {
    Ignored,
    Consumed,
};

class IIntentHandler {
public:
    virtual ~IIntentHandler() = default;

    virtual IntentResult HandleIntent(const Intent& intent) = 0;
};

}  // namespace cave