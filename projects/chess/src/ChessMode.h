#pragma once
#include <cave/runtime/gameplay/IGameMode.h>

namespace cave {

class ChessGameMode final : public cave::IGameMode {
public:
    std::string_view GetId() const final { return "chess"; }

    void OnEnter(GameSession& p_session) final;

    void OnExit(GameSession& p_session) final;

    void Tick(GameSession& p_session, const GameFrameTime& p_time) final;
};

}  // namespace cave
