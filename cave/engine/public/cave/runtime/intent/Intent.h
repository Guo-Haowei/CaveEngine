// =============================================================================
// File: engine/public/cave/framework/intent/Intent.h
// =============================================================================
#pragma once
#include <string_view>
#include <string>

#include "cave/core/string/StringId.h"

namespace cave {

using IntentTypeId = StringId;

class Intent {
public:
    virtual ~Intent() = default;

    virtual IntentTypeId GetTypeId() const = 0;
    virtual std::string_view GetDebugName() const = 0;

#if USING(DEBUG_BUILD)
    virtual std::string DebugString() const { return ""; }
#endif
};

#define CAVE_DECLARE_INTENT(STR)                                       \
public:                                                                \
    inline static constexpr ::cave::StringId TypeId{ STR };            \
    ::cave::IntentTypeId GetTypeId() const override { return TypeId; } \
    std::string_view GetDebugName() const override { return TypeId.DebugName(); }

}  // namespace cave