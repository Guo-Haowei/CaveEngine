#pragma once
#include "cave/core/Option.h"

#include "Piece.h"
#include "Square.h"

namespace chess::core {

class CheckerList {
    struct Val {
        Square square;
        PieceType type;
    };

public:
    CheckerList();

    void Clear();

    cave::Option<Val> Get(int p_idx) const;

    bool Add(Square p_square, PieceType p_type);

    uint8_t Count() const { return m_count; }

private:
    std::array<cave::Option<Val>, 2> m_squares;
    uint8_t m_count;
};

}  // namespace chess::core
