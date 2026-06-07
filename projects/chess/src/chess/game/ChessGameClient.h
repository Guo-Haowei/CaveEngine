#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/core/Position.h"
#include "chess/presentation/ChessPresenter.h"

// clang-format off
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

class ChessGameSession;
class ChessMatchAuthority;

class ChessGameClient : public cave::IIntentHandler {
public:
    ChessGameClient(cave::IHostServices& p_host,
                    ChessGameSession& p_session,
                    ChessMatchAuthority& p_auth);
    ~ChessGameClient();

    void OnBoot();

    void Present();

    std::span<const core::Move> LegalMovesFromSquare(core::Square p_sq);

    const core::Position& Replica() const { return m_replica; }

    ChessPresenter& Presenter() { return m_presenter; }

    bool HandleIntent(cave::Intent& p_intent) override;

    cave::DebugId debugId() const override { return m_debug_id; }

private:
    void OnMoveCommitted(core::Move p_mv);
    void OnMoveRejected(core::Move p_mv);

    void OnPositionChange();

    void ResetBoard();

    ChessMatchAuthority& m_auth;
    ChessGameSession& m_session;
    ChessPresenter m_presenter;

    core::Position m_replica;  // replicated position of auth

    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;

    cave::IHostServices& m_host;
    cave::IntentDispatcher& m_intent;
    const cave::DebugId m_debug_id;
};

}  // namespace chess