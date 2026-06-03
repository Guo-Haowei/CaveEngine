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
    m_replica = Position::Startpos();

    OnPositionChange();
}

void ChessGameClient::OnBoot(cave::IHostServices& p_host) {
    m_presenter.OnBoot(p_host.SceneQuery());
    ResetBoard();

    m_presenter.RedrawPosition(p_host, m_replica);
}

void ChessGameClient::Tick(cave::IHostServices& p_host) {
    AuthorityEvent e;
    while (m_auth.Pop(e)) {
        using namespace cave;

        switch (e.type) {
            case AuthorityEventType::MoveCommitted: {
                core::UndoState undo;
                m_replica.MakeMove(e.move, undo);
                OnPositionChange();
                // redraw board
                m_presenter.RedrawPosition(p_host, m_replica);
            } break;
            case AuthorityEventType::MoveRejected: {
                p_host.Log().Info(LogChannel::Game, "Invalid move!");
            } break;
            case AuthorityEventType::GameOver: {
            } break;
            default: {
                assert(0);
            } break;
        }
    }

    PresentationContext ctx{
        .host = p_host,
    };

    m_presenter.Present(ctx);
}

void ChessGameClient::OnPositionChange() {
    const core::MoveList moves = MoveGen::LegalMove(m_replica);

    m_move_cache.clear();
    for (Move move : moves) {
        m_move_cache[move.From()].push_back(move);
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
