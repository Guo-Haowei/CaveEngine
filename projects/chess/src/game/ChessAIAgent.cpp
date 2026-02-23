#include "ChessAIAgent.h"

#include <random>

#include "core/MoveGen.h"
#include "ChessGameClient.h"
#include "ChessMatchAuthority.h"

namespace chess {

using core::Move;
using core::MoveGen;
using core::Position;

void ChessAIAgent::Tick() {
    const Position& replica = m_client.Replica();
    const bool my_turn = std::to_underlying(replica.SideToMove()) == m_player;
    if (!my_turn) return;

    auto& inbox = m_auth.Inbox(m_player);
    inbox.Clear();

    const core::MoveList moves = MoveGen::LegalMove(replica);

    static std::mt19937 rng(std::random_device{}());

    const uint32_t count = moves.Size();
    if (count) {
        std::uniform_int_distribution<uint32_t> dist(0, count - 1);

        const Move move = moves[dist(rng)];

        PlayerIntent intent = {
            IntentType::AttemptMove,
            move,
        };
        inbox.Push(intent);
    }
}

}  // namespace chess
