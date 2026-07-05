#pragma once
#include <cstdint>
#include <tuple>

namespace chess::core {

class Square {
public:
    enum {
        kInvalid = 64
    };

    constexpr Square() noexcept
        : index_(kInvalid) {}

    explicit constexpr Square(uint8_t index) noexcept
        : index_(index) {
    }

    static constexpr Square fromFileRank(uint8_t file, uint8_t rank) {
        const uint8_t val = rank * 8 + file;
        return Square(val);
    }

    bool isValid() const { return index_ < kInvalid; }

    constexpr uint8_t index() const { return index_; }

    uint8_t file() const { return index_ & 7; }
    uint8_t rank() const { return index_ >> 3; }
    auto fileRank() const -> std::tuple<uint8_t, uint8_t> {
        return std::make_tuple(file(), rank());
    }

    std::strong_ordering operator<=>(const Square&) const = default;

    // only returns true if square is between A and B
    bool sameLineInclusive(Square a, Square b) const;

    const char* uci() const;

    static const Square A1;
    static const Square B1;
    static const Square C1;
    static const Square D1;
    static const Square E1;
    static const Square F1;
    static const Square G1;
    static const Square H1;

    static const Square A2;
    static const Square B2;
    static const Square C2;
    static const Square D2;
    static const Square E2;
    static const Square F2;
    static const Square G2;
    static const Square H2;

    static const Square A3;
    static const Square B3;
    static const Square C3;
    static const Square D3;
    static const Square E3;
    static const Square F3;
    static const Square G3;
    static const Square H3;

    static const Square A4;
    static const Square B4;
    static const Square C4;
    static const Square D4;
    static const Square E4;
    static const Square F4;
    static const Square G4;
    static const Square H4;

    static const Square A5;
    static const Square B5;
    static const Square C5;
    static const Square D5;
    static const Square E5;
    static const Square F5;
    static const Square G5;
    static const Square H5;

    static const Square A6;
    static const Square B6;
    static const Square C6;
    static const Square D6;
    static const Square E6;
    static const Square F6;
    static const Square G6;
    static const Square H6;

    static const Square A7;
    static const Square B7;
    static const Square C7;
    static const Square D7;
    static const Square E7;
    static const Square F7;
    static const Square G7;
    static const Square H7;

    static const Square A8;
    static const Square B8;
    static const Square C8;
    static const Square D8;
    static const Square E8;
    static const Square F8;
    static const Square G8;
    static const Square H8;

private:
    uint8_t index_;
};

Square EnpassantCapturedSquare(Square from, Square to);

}  // namespace chess::core

namespace std {

template<>
struct hash<chess::core::Square> {
    std::size_t operator()(const chess::core::Square& square) const {
        return std::hash<uint8_t>{}(square.index());
    }
};

}  // namespace std
