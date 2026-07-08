// =============================================================================
// File: cave/core/ids/GenId.h
// =============================================================================
#pragma once
#include <cstdint>
#include <format>
#include <string>
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

    bool valid() const {
        return gen != kInvalidGen;
    }

    bool operator==(const GenId<Tag>& rhs) const {
        return index == rhs.index && gen == rhs.gen;
    }

    bool operator!=(const GenId<Tag>& rhs) const {
        return index != rhs.index || gen != rhs.gen;
    }

    std::string toString() const {
        return std::format("id=({},{})", index, gen);
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
