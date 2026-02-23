#include "CheckerList.h"

#include <cassert>

namespace chess::core {

CheckerList::CheckerList() {
    Clear();
}

void CheckerList::Clear() {
    m_count = 0;
    m_squares[0] = cave::None();
    m_squares[1] = cave::None();
}

bool CheckerList::Add(Square p_square, PieceType p_type) {
    if (m_count == 2) return false;

    m_squares[m_count++] = cave::Some(Val{ p_square, p_type });
    return true;
}

cave::Option<CheckerList::Val> CheckerList::Get(int p_idx) const {
    assert(p_idx < 2);
    return m_squares[p_idx];
}

}  // namespace chess::core
