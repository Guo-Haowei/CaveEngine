#pragma once
#include <functional>
#include <span>

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/agents/IPlayerAgent.h"
#include "chess/core/Move.h"

// clang-format off
namespace cave { class GridSelectController; }
namespace cave { class IGameInput; }
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

class ChessGameClient;
class ChessPresenter;
class LocalHumanAgent;

class ChessGridSelectorAdapter {
    using Entity = cave::ecs::Entity;
    using GetPlayerFunc = std::function<LocalHumanAgent*(PlayerId)>;

public:
    ChessGridSelectorAdapter(cave::IHostServices& host,
                             ChessGameClient& game,
                             ChessPresenter& presenter) noexcept;

    bool canSelect(int x, int y);
    void onSelect(int x, int y);
    bool canDrop(int sx, int sy, int dx, int dy);
    void onDrop(int sx, int sy, int dx, int dy);
    void onCancel();
    void onInvalid(int sx, int sy, int dx, int dy);

    void tick();

    void setController(cave::GridSelectController* controller) {
        controller_ = controller;
    }

    void setPlayerCb(GetPlayerFunc&& func) {
        get_player_cb_ = std::move(func);
    }

private:
    void tickPointer(const cave::IGameInput& input);
    void tickKeyboard(const cave::IGameInput& input);

    cave::IHostServices& host_;
    cave::GridSelectController* controller_{};
    Entity camera_id_;

    ChessGameClient& client_;
    ChessPresenter& presenter_;

    GetPlayerFunc get_player_cb_;
};

}  // namespace chess
