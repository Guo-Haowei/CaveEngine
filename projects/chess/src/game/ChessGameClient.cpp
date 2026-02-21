#include "ChessGameClient.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IInputService.h"
#include "core/MoveGen.h"
#include "ChessMatchAuthority.h"

namespace chess {

using core::Move;
using core::MoveGen;
using core::Position;

ChessGameClient::ChessGameClient(ChessMatchAuthority& p_auth)
    : m_presenter{}
    , m_auth{ p_auth } {
}

void ChessGameClient::ResetBoard() {
    m_replica = Position::Default();

    OnPositionChange();
}

void ChessGameClient::OnGameBegin(cave::IHostServices& p_host) {
    m_presenter.OnGameBegin(p_host.SceneQuery());
    ResetBoard();
}

void ChessGameClient::OnGameEnd(cave::IHostServices& p_host) {
    unused(p_host);
    m_presenter.OnGameEnd();
}

void ChessGameClient::Tick(cave::IHostServices& p_host) {
    AuthorityEvent e;
    while (m_auth.Pop(e)) {
        switch (e.type) {
            case AuthorityEventType::MoveCommitted: {
                core::UndoState undo;
                m_replica.MakeMove(e.move, undo);
                OnPositionChange();
            } break;
            default: {
                assert(0);
            } break;
        }
    }

    PresentationContext ctx{ p_host };
    m_presenter.Present(ctx);
}

void ChessGameClient::OnPositionChange() {
    MoveGen::Pseudo(m_replica, m_moves);

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
