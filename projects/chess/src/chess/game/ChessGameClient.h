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
    ChessGameClient(cave::IHostServices& host,
                    ChessGameSession& session,
                    ChessMatchAuthority& auth);
    ~ChessGameClient();

    void onBoot();

    void present();

    std::span<const core::Move> legalMoves(core::Square square) const;

    const core::Position& replica() const { return replica_; }

    ChessPresenter& presenter() { return presenter_; }

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return debug_id_; }

private:
    void onMoveCommitted(core::Move move);
    void onMoveRejected(core::Move move);

    void onPositionChange();

    void resetBoard();

    ChessMatchAuthority& auth_;
    ChessGameSession& session_;
    ChessPresenter presenter_;

    core::Position replica_;

    cave::IHostServices& host_;
    cave::IntentDispatcher& intent_dispatcher_;
    const cave::DebugId debug_id_;

    std::unordered_map<core::Square, std::vector<core::Move>> move_cache_;
};

}  // namespace chess