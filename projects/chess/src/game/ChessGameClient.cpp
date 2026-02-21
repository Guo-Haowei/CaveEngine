#include "ChessGameClient.h"

#include "core/MoveGen.h"

namespace chess {

using core::Position;
using core::Move;
using core::MoveGen;

void ChessGameClient::ResetBoard() {
    m_pos = Position::Default();

    OnPositionChange();
}

void ChessGameClient::OnPositionChange() {
    MoveGen::Pseudo(m_pos, m_moves);

    m_move_cache.clear();
    for (Move mv : m_moves) {
        m_move_cache[mv.from].push_back(mv);
    }
}

std::span<const core::Move> ChessGameClient::LegalMovesFromSquare(core::Square p_sq) {
    auto it = m_move_cache.find(p_sq);
    if (it == m_move_cache.end()) {
        return {};
    }

    return it->second;
}

}  // namespace chess
