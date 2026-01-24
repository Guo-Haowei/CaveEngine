#pragma once

#if USING(DEBUG_BUILD)
#define STRING_ID_KEEKP_SOURCE IN_USE
#else
#define STRING_ID_KEEKP_SOURCE NOT_IN_USE
#endif

namespace cave {

static constexpr inline uint64_t fnv1a_64(std::string_view s) {
    uint64_t h = 14695981039346656037ull;  // offset basis
    for (unsigned char c : s) {
        h ^= (uint64_t)c;
        h *= 1099511628211ull;  // FNV prime
    }
    return h;
}

class StringId {
public:
    constexpr StringId() = default;

    constexpr StringId(std::string_view p_str)
        : m_hash(fnv1a_64(p_str))
#if USING(STRING_ID_KEEKP_SOURCE)
        , m_source(p_str)
#endif
    {
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
    std::string m_source;
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
