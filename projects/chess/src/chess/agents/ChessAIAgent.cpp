#include "ChessAIAgent.h"

#include <random>

#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/core/MoveGen.h"
#include "chess/game/ChessGameClient.h"
#include "chess/game/ChessIntent.h"
#include "chess/game/ChessMatchAuthority.h"

namespace chess {

using core::Move;
using core::MoveGen;
using core::Position;

void ChessAIAgent::tick(cave::IHostServices& host) {
    const Position& replica = client_.replica();
    const bool my_turn = replica.SideToMove() == playerId();
    if (!my_turn) {
        return;
    }

    const core::MoveList moves = MoveGen::LegalMove(replica);

    static std::mt19937 rng(std::random_device{}());

    const uint32_t count = moves.Size();
    if (count) {
        std::uniform_int_distribution<uint32_t> dist(0, count - 1);

        const uint32_t idx = dist(rng);
        assert(idx < count);
        const Move move = moves[idx];

        host.intentDispatcher().Queue<ChessMoveIntent>(playerId(), move);
    }
}

}  // namespace chess
