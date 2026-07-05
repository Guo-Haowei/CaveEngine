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

class ChessBoardView;
class ChessGameClient;
class LocalHumanAgent;

class ChessGridSelectorAdapter {
    using Entity = cave::ecs::Entity;
    using GetPlayerFunc = std::function<LocalHumanAgent*(core::Color)>;

public:
    ChessGridSelectorAdapter(cave::IHostServices& host,
                             ChessGameClient& game,
                             ChessBoardView& board_view) noexcept;

    bool canSelect(int x, int y);
    void onSelect(int x, int y);
    bool canDrop(int sx, int sy, int dx, int dy);
    void onDrop(int sx, int sy, int dx, int dy);
    void onCancel();
    void onInvalid(int sx, int sy, int dx, int dy);

    void tick();

    void setController(cave::GridSelectController* controller) {
        m_controller = controller;
    }

    void setPlayerCb(GetPlayerFunc&& func) {
        m_get_player_func = std::move(func);
    }

private:
    void tickPointer(const cave::IGameInput& input);
    void tickKeyboard(const cave::IGameInput& input);

    cave::IHostServices& m_host_;
    cave::GridSelectController* m_controller{};
    Entity m_camera_id;

    ChessGameClient& m_client;
    ChessBoardView& m_board_view;

    GetPlayerFunc m_get_player_func;
};

}  // namespace chess
