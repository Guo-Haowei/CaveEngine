// =============================================================================
// File: cave/core/containers/StringHash.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"

namespace cave {

struct TransparentStringHash {
    using is_transparent = void;

    size_t operator()(std::string_view value) const noexcept {
        return std::hash<std::string_view>{}(value);
    }

    size_t operator()(const std::string& value) const noexcept {
        return operator()(std::string_view{ value });
    }

    size_t operator()(const char* value) const noexcept {
        return operator()(std::string_view{ value });
    }
};

template<typename T,
         typename Allocator = std::allocator<std::pair<const String, T>>>
using StringHashMap = HashMap<
    String,
    T,
    TransparentStringHash,
    std::equal_to<>,
    Allocator>;

template<typename T,
         typename Allocator = std::allocator<std::pair<const String, T>>>
using StringMap = Map<String, T, std::less<>, Allocator>;

}  // namespace cave
