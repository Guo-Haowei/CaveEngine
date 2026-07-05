// =============================================================================
// File: cave/core/ids/Guid.h
// =============================================================================
#pragma once
#include <array>
#include <compare>
#include <string>

#include "cave/core/Option.h"

namespace cave {

class Guid {
public:
    static constexpr size_t kSize = 16;

    Guid() = default;

    static Guid null() {
        return Guid{};
    }

    static Guid make();
    static Option<Guid> parse(const char* start, size_t length);

    static Option<Guid> parse(std::string_view sv) {
        return parse(sv.data(), sv.size());
    }

    static Option<Guid> parse(const std::string& str) {
        return parse(str.c_str(), str.length());
    }

    bool isNull() const { return *this == Guid{}; }

    bool operator==(const Guid& rhs) const { return m_data == rhs.m_data; }
    bool operator!=(const Guid& rhs) const { return m_data != rhs.m_data; }
    std::strong_ordering operator<=>(const Guid& rhs) const noexcept {
        return m_data <=> rhs.m_data;
    }

    const uint8_t* data() const { return m_data.data(); };

    std::string toString() const;

private:
    std::array<uint8_t, kSize> m_data{};
};

}  // namespace cave

namespace std {

template<>
struct hash<cave::Guid> {
    std::size_t operator()(const cave::Guid& guid) const {
        std::size_t hash = 0;
        const uint8_t* data = guid.data();
        // Combine hash for each byte in the buffer
        for (std::size_t i = 0; i < sizeof(cave::Guid); ++i) {
            hash ^= std::hash<uint8_t>{}(data[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }
};

}  // namespace std
