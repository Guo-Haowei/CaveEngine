#include "LocalHumanAgent.h"

#include "ChessMatchAuthority.h"

namespace chess {

void LocalHumanAgent::Tick() {
    PlayerIntent i;
    while (m_local_inbox.Pop(i)) {
        m_auth.Inbox(m_player).Push(i);
    }
}

}  // namespace chess
