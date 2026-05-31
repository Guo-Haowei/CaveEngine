// =============================================================================
// File: engine/public/cave/framework/intent/Intent.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/core/string/StringId.h"

namespace cave {

using IntentTypeId = StringId;

class Intent {
public:
    virtual ~Intent() = default;

    virtual IntentTypeId GetTypeId() const = 0;
    virtual std::string_view GetDebugName() const = 0;
};

}  // namespace cave