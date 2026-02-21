#include "LocalHumanAgent.h"

#include "ChessMatchAuthority.h"

namespace chess {

void LocalHumanAgent::Tick() {
    // Forward everything into authority
    PlayerIntent i;
    while (m_local_inbox.Pop(i)) {
        m_authority.Inbox(m_player).Push(i);
    }
}

}  // namespace chess
