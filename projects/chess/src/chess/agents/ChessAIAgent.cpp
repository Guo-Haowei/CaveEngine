#include "ChessAIAgent.h"

#include <random>

#include "core/MoveGen.h"
#include "ChessGameClient.h"
#include "ChessIntent.h"
#include "ChessMatchAuthority.h"

#include "cave/runtime/intent/IntentDispatcher.h"

namespace chess {

using core::Move;
using core::MoveGen;
using core::Position;

void ChessAIAgent::Tick(cave::IHostServices& p_host) {
    const Position& replica = m_client.Replica();
    const bool my_turn = std::to_underlying(replica.SideToMove()) == m_player;
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

        p_host.Intent().Queue<ChessMoveIntent>(m_player, move);
    }
}

}  // namespace chess
