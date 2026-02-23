#pragma once
#include <cstdint>
#include <tuple>

namespace chess::core {

class Square {
public:
    constexpr Square() noexcept
        : m_index(64) {}

    explicit constexpr Square(uint8_t p_index) noexcept
        : m_index(p_index) {
    }

    static constexpr Square FromFileRank(uint8_t file, uint8_t rank) {
        const uint8_t val = rank * 8 + file;
        return Square(val);
    }

    bool IsValid() const { return m_index < 64; }

    constexpr uint8_t Index() const { return m_index; }

    std::tuple<uint8_t, uint8_t> FileRank() const;

    std::strong_ordering operator<=>(const Square&) const = default;

    // only returns true if square is between A and B
    bool SameLineInclusive(Square a, Square b) const;

    const char* ToString() const;

private:
    uint8_t m_index;
};

}  // namespace chess::core

namespace std {

template<>
struct hash<chess::core::Square> {
    std::size_t operator()(const chess::core::Square& p_sq) const {
        return std::hash<uint8_t>{}(p_sq.Index());
    }
};

}  // namespace std

