#include "ChessGameClient.h"

#include "ChessIntent.h"
#include "ChessMatchAuthority.h"
#include "core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IInputService.h"

namespace chess {

using core::Move;
using core::MoveGen;
using core::Position;

ChessGameClient::ChessGameClient(cave::IHostServices& p_host, ChessMatchAuthority& p_auth)
    : m_presenter{}
    , m_auth(p_auth)
    , m_host(p_host)
    , m_intent(p_host.Intent())
    , m_debug_id(cave::MakeDebugId(this)) {

    m_intent.AddHandler<AuthMoveCommitted>(this);
    m_intent.AddHandler<AuthMoveRejected>(this);
    m_intent.AddHandler<AuthGameOver>(this);
}

ChessGameClient::~ChessGameClient() {
    m_intent.RemoveHandler<AuthMoveCommitted>(this);
    m_intent.RemoveHandler<AuthMoveRejected>(this);
    m_intent.RemoveHandler<AuthGameOver>(this);
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

bool ChessGameClient::HandleIntent(cave::Intent& p_intent) {
    if (auto i = dynamic_cast<AuthMoveCommitted*>(&p_intent)) {
        core::UndoState undo;
        m_replica.MakeMove(i->mv, undo);
        OnPositionChange();
        // redraw board
        m_presenter.RedrawPosition(m_host, m_replica);
        return true;
    }

    if (auto i = dynamic_cast<AuthMoveRejected*>(&p_intent)) {
        m_host.Log().Info(cave::LogChannel::Game, "Invalid move!");
        return true;
    }

    if (auto i = dynamic_cast<AuthGameOver*>(&p_intent)) {
        m_host.Log().Info(cave::LogChannel::Game, "Game over!");
        return true;
    }

    return false;
}

void ChessGameClient::Tick(cave::IHostServices& p_host) {
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
