// =============================================================================
// File: public/cave/runtime/core/debugger/DebugId.h
// =============================================================================
#pragma once
#include <cstdint>
#include <string_view>

namespace cave {

struct DebugId {
    uint64_t uid;
    std::string_view type;
};

}  // namespace cave
