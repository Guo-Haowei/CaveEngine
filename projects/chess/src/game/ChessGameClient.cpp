#include "ChessGameClient.h"

#include "ChessGameSession.h"
#include "ChessIntent.h"
#include "ChessMatchAuthority.h"
#include "core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IInputService.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using namespace cave;
using namespace cave::literals;
using core::Move;
using core::MoveGen;
using core::Position;

ChessGameClient::ChessGameClient(cave::IHostServices& p_host,
                    ChessGameSession& p_session,
                                 ChessMatchAuthority& p_auth)
    : m_presenter(p_host)
    , m_session(p_session)
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

void ChessGameClient::OnBoot() {
    m_presenter.OnBoot(m_host.SceneQuery());
    ResetBoard();

    m_presenter.InitBoard(m_replica);
}

bool ChessGameClient::HandleIntent(cave::Intent& p_intent) {
    if (auto intent = dynamic_cast<AuthMoveCommitted*>(&p_intent)) {
        OnMoveCommitted(intent->mv);
        return true;
    }

    if (auto intent = dynamic_cast<AuthMoveRejected*>(&p_intent)) {
        OnMoveRejected(intent->mv);
        return true;
    }

    if (auto intenti = dynamic_cast<AuthGameOver*>(&p_intent)) {
        m_host.Log().Info(cave::LogChannel::Game, "Game over!");
        return true;
    }

    return false;
}

void ChessGameClient::OnMoveCommitted(core::Move p_mv) {
    m_presenter.ApplyMove(p_mv);

    core::UndoState undo;
    m_replica.MakeMove(p_mv, undo);
    OnPositionChange();

    m_session.SetState(SessionState::ResolvingMove);
}

void ChessGameClient::OnMoveRejected(core::Move p_mv) {
    m_host.Log().Info(cave::LogChannel::Game, "Invalid move!");
}

void ChessGameClient::Present() {
    m_presenter.Present();
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
