#pragma once
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/script/native/NativeScript.h"

#include "chess/game/ChessGameMode.h"

namespace chess {

class ChessGameMode;

class BoardController final : public cave::NativeScript {
public:
    BoardController();
    ~BoardController() override;

    void alwaysRun(cave::SceneContext& ctx,
                   cave::SceneCommandWriter& writer) override;
    void start() override;
    void destroy() override;

    void update(float dt) override;

private:
    cave::IntentBus m_intent_bus;
    std::unique_ptr<ChessGameMode> m_game;
};

}  // namespace chess
