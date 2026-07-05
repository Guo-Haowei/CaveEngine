#pragma once
#include "cave/runtime/script/native/NativeScript.h"

#include "chess/game/ChessGameMode.h"

namespace chess {

class ChessGameMode;

class BoardController final : public cave::NativeScript {
public:
    BoardController();
    ~BoardController() override;

    void onCreate(cave::SceneContext& ctx) override;
    void onDestroy() override;

    void onUpdate(cave::SceneContext& ctx, float dt) override;

private:
    std::unique_ptr<ChessGameMode> m_game;
};

}  // namespace chess
