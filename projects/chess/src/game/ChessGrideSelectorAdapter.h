#pragma once
#include <span>
#include "core/Move.h"
#include "IPlayerAgent.h"

// clang-format off
namespace cave { class GridSelectController; }
namespace cave { class IInputService; }
// clang-format on

namespace chess {

class ChessGameClient;
class ChessPresenter;
class LocalHumanAgent;

class ChessGridSelectorAdapter {
public:
    explicit ChessGridSelectorAdapter(
        ChessGameClient& p_game,
        ChessPresenter& p_presenter) noexcept
        : m_client(p_game)
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

    void SetPlayer(PlayerId p_player_id, LocalHumanAgent* p_player) {
        m_players[p_player_id] = p_player;
    }

private:
    ChessGameClient& m_client;

    // @TODO: do not pass presenter here, instead query highlight
    ChessPresenter& m_presenter;

    cave::GridSelectController* m_controller;
    LocalHumanAgent* m_players[2]{ nullptr, nullptr };
};

}  // namespace chess
