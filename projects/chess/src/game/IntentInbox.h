#pragma once
#include <deque>
#include "ChessTypes.h"

namespace chess {

class IntentInbox {
public:
    void Push(const PlayerIntent& p_intent);

    bool Pop(PlayerIntent& p_out);

    void Clear();

private:
    std::deque<PlayerIntent> m_deque;
};

}  // namespace chess
