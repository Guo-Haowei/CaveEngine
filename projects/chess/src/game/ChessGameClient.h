#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include "core/Position.h"
#include "ChessPresenter.h"

#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentDispatcher.h"

// clang-format off
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

class ChessMatchAuthority;

enum class ChessClientState : uint8_t {
    Idle,
    PendingPromotion,
    AnimatingMove,
};

class ChessGameClient : public cave::IIntentHandler {
public:
    ChessGameClient(cave::IHostServices& p_host, ChessMatchAuthority& p_auth);
    ~ChessGameClient();

    void OnBoot();

    void Present();
    void SyncState();

    std::span<const core::Move> LegalMovesFromSquare(core::Square p_sq);

    const core::Position& Replica() const { return m_replica; }

    ChessPresenter& Presenter() { return m_presenter; }

    bool CanAcceptMoveInput() const {
        return m_state == ChessClientState::Idle;
    }

    bool IsAnimating() const {
        return m_state == ChessClientState::AnimatingMove;
    }

    bool HandleIntent(cave::Intent& p_intent) override;

    cave::DebugId GetDebugId() const override { return m_debug_id; }

private:
    void OnMoveCommitted(core::Move p_mv);
    void OnMoveRejected(core::Move p_mv);

    void OnPositionChange();
    void SetState(ChessClientState p_state);

    void ResetBoard();

    ChessMatchAuthority& m_auth;
    ChessPresenter m_presenter;

    core::Position m_replica;  // replicated position of auth

    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;
    ChessClientState m_state = ChessClientState::Idle;

    cave::IHostServices& m_host;
    cave::IntentDispatcher& m_intent;
    const cave::DebugId m_debug_id;
};

}  // namespace chess