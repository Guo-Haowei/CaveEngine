#include "Bitboard.h"

#include "cave/core/ErrorMacros.h"

namespace cave::chess {

bool Bitboard::Test(Square p_sq) const {
    DEV_ASSERT_MSG(p_sq.IsValid(), "square out of bounds");
    return m_val & (1llu << p_sq.val);
}

void Bitboard::Set(Square p_sq) {
    DEV_ASSERT_MSG(p_sq.IsValid(), "square out of bounds");
    m_val |= (1llu << p_sq.val);
}

}  // namespace cave::chess
