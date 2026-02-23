#include "CheckerList.h"

#include <cassert>

namespace chess::core {

CheckerList::CheckerList()
    : m_squares{ cave::None(), cave::None() }
    , m_count{ 0 } {
}

bool CheckerList::Add(Square p_square, PieceType p_type) {
    if (m_count == 2) return false;

    m_squares[m_count++] = cave::Some(Val{ p_square, p_type });
    return true;
}

auto CheckerList::Get(int p_idx) const {
    assert(p_idx < 2);
    return m_squares[p_idx];
}

}  // namespace chess::core
