#pragma once
#include <functional>
#include <span>
#include "core/Move.h"
#include "IPlayerAgent.h"

#include "cave/runtime/intent/IntentDispatcher.h"

// clang-format off
namespace cave { class GridSelectController; }
namespace cave { class IInputService; }
// clang-format on

namespace chess {

class ChessGameClient;
class ChessPresenter;
class LocalHumanAgent;

class ChessGridSelectorAdapter {
    using GetPlayerFunc = std::function<LocalHumanAgent*(PlayerId)>;

public:
    explicit ChessGridSelectorAdapter(cave::IntentDispatcher& p_intent,
                                      ChessGameClient& p_game,
                                      ChessPresenter& p_presenter) noexcept
        : m_intent(p_intent)
        , m_client(p_game)
        , m_presenter(p_presenter) {
    }

    bool CanSelect(int x, int y);
    void OnSelect(int x, int y);
    bool CanDrop(int sx, int sy, int dx, int dy);
    void OnDrop(int sx, int sy, int dx, int dy);
    void OnCancel();
    void OnInvalid(int sx, int sy, int dx, int dy);

    void Tick(cave::IInputService& p_input);

    void SetController(cave::GridSelectController* p_controller) {
        m_controller = p_controller;
    }

    void SetGetPlayerFunc(GetPlayerFunc&& p_func) {
        m_get_player_func = std::move(p_func);
    }

private:
    cave::IntentDispatcher& m_intent;
    ChessGameClient& m_client;

    // @TODO: do not pass presenter here, instead query highlight
    ChessPresenter& m_presenter;

    cave::GridSelectController* m_controller;
    GetPlayerFunc m_get_player_func;
};

}  // namespace chess
