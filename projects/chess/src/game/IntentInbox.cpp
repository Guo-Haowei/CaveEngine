#include "IntentInbox.h"

namespace chess {

void IntentInbox::Push(const PlayerIntent& p_intent) {
    m_deque.push_back(p_intent);
}

bool IntentInbox::Pop(PlayerIntent& p_out) {
    if (m_deque.empty()) {
        return false;
    }

    p_out = m_deque.front();
    m_deque.pop_front();
    return true;
}

void IntentInbox::Clear() {
    m_deque.clear();
}

}  // namespace chess
