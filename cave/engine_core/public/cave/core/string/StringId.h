// =============================================================================
// File: cave/core/string/StringId.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"
#include "cave/core/hash/Hash.h"
#include "cave/core/containers/FixedString.h"

#define STRING_ID_KEEP_SOURCE IN_USE

namespace cave {

class StringId {
public:
    explicit constexpr StringId() = default;

    explicit constexpr StringId(std::string_view sv)
        : hash_(Hash::hash64(sv)) {
#if USING(STRING_ID_KEEP_SOURCE)
        debug_name_.assign(sv);
#endif
    }

    constexpr auto operator<=>(const StringId& rhs) const {
        return hash_ <=> rhs.hash_;
    }

#if USING(STRING_ID_KEEP_SOURCE)
    std::string_view debugName() const { return debug_name_.view(); }

    bool operator==(const StringId& p_other) const;
#else
    std::string_view debugName() const { return ""; }

    constexpr bool operator==(const StringId& rhs) const {
        return hash_ == rhs.hash_;
    }
#endif

    constexpr uint64_t hash() const {
        return hash_;
    }

private:
    uint64_t hash_{ 0 };
#if USING(STRING_ID_KEEP_SOURCE)
    FixedString<32> debug_name_;
#endif
};

namespace literals {

constexpr StringId operator"" _sid(const char* str, std::size_t len) {
    return StringId{ std::string_view{ str, len } };
}

}  // namespace literals

}  // namespace cave

namespace std {
template<>
struct hash<::cave::StringId> {
    size_t operator()(const ::cave::StringId& str_id) const noexcept {
        return str_id.hash();
    }
};

}  // namespace std
