// =============================================================================
// File: public/cave/runtime/core/GenId.h
// =============================================================================
#pragma once
#include <cstdint>
#include <type_traits>

namespace cave {

template<typename Tag>
struct GenId {
    enum : uint32_t {
        kInvalidGen = 0,
        kInitialGen = 1,
    };

    uint32_t index{};
    uint32_t gen{};

    bool IsValid() const {
        return gen != kInvalidGen;
    }

    bool operator==(const GenId<Tag>& p_other) const {
        return index == p_other.index && gen == p_other.gen;
    }

    bool operator!=(const GenId<Tag>& p_other) const {
        return index != p_other.index || gen != p_other.gen;
    }
};

}  // namespace cave

namespace std {

template<typename T>
struct hash<::cave::GenId<T>> {
    size_t operator()(const ::cave::GenId<T>& id) const noexcept {
        return (static_cast<std::size_t>(id.gen) << 32) | id.index;
    }
};

}  // namespace std
