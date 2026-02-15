// =============================================================================
// File: public/cave/core/string/StringId.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"
#include "cave/core/hash/Hash.h"
#include "cave/core/containers/FixedString.h"

#define STRING_ID_KEEKP_SOURCE IN_USE

namespace cave {

class StringId {
public:
    explicit constexpr StringId() = default;

    explicit constexpr StringId(std::string_view p_str)
        : m_hash(Hash::Hash64(p_str)) {
#if USING(STRING_ID_KEEKP_SOURCE)
        m_debug.assign(p_str);
#endif
    }

    constexpr auto operator<=>(const StringId& p_other) const {
        return m_hash <=> p_other.m_hash;
    }

#if USING(STRING_ID_KEEKP_SOURCE)
    bool operator==(const StringId& p_other) const;
#else
    constexpr bool operator==(const StringId& p_other) const {
        return m_hash == p_other.m_hash;
    }
#endif

    constexpr uint64_t GetHash() const {
        return m_hash;
    }

private:
    uint64_t m_hash{ 0 };
#if USING(STRING_ID_KEEKP_SOURCE)
    FixedString<32> m_debug;
#endif
};

}  // namespace cave

namespace std {
template<>
struct hash<::cave::StringId> {
    size_t operator()(const ::cave::StringId& p_str_id) const noexcept {
        return p_str_id.GetHash();
    }
};

}  // namespace std
