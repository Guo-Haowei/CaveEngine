#include "ChessGameClient.h"

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

ChessGameClient::ChessGameClient(cave::IHostServices& p_host, ChessMatchAuthority& p_auth)
    : m_presenter(p_host)
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

static const char* ToString(ChessClientState p_state) {
    switch (p_state) {
        case ChessClientState::Idle:
            return "Idle";
        case ChessClientState::PendingPromotion:
            return "PendingPromotion";
        case ChessClientState::AnimatingMove:
            return "AnimatingMove";
        default:
            return "?";
    }
}

void ChessGameClient::SetState(ChessClientState p_state) {
    if (p_state == m_state) return;

    m_host.Log().Trace(LogChannel::Game,
                       std::format("ChessState {} -> {}", ToString(m_state), ToString(p_state)));

    m_state = p_state;
}

void ChessGameClient::OnMoveCommitted(core::Move p_mv) {
    m_presenter.ApplyMove(p_mv);

    core::UndoState undo;
    m_replica.MakeMove(p_mv, undo);
    OnPositionChange();

    SetState(ChessClientState::AnimatingMove);
}

void ChessGameClient::OnMoveRejected(core::Move p_mv) {
    m_host.Log().Info(cave::LogChannel::Game, "Invalid move!");
}

void ChessGameClient::SyncState() {
    auto& query = m_host.SceneQuery();
    const bool no_animation = query.GetComponentCount(TransformAnimationComponent_Id) == 0;

    // @TODO: refactor this part
    if (no_animation) {
        SetState(ChessClientState::Idle);
    } else {
        SetState(ChessClientState::AnimatingMove);
    }
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
