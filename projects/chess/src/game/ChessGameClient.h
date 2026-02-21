#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include "core/Position.h"
#include "ChessGrideSelectorAdapter.h"
#include "ChessPresenter.h"

#include "cave/runtime/controller/GridSelectController.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class IInputService; }
// clang-format on

namespace chess {

class ChessGameClient {
public:
    ChessGameClient();

    void OnGameBegin(cave::IHostServices& p_host);

    void OnGameEnd(cave::IHostServices& p_host);

    void Tick(cave::IHostServices& p_host);

    std::span<const core::Move> LegalMoves() const { return m_moves; }

    std::span<const core::Move> LegalMovesFromSquare(core::Square p_sq);

    const core::Position& Pos() const { return m_replicated; }

private:
    void OnPositionChange();

    // @TODO: fix
    void ProcessInput(cave::IInputService& p_input);

    void ResetBoard();

    core::Position m_replicated;  // replicated position of auth

    core::MoveList m_moves;
    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;

    ChessPresenter m_presenter;
    ChessGridSelectorAdapter m_grid_adapter;
    std::unique_ptr<cave::GridSelectController> m_selector;
};

}  // namespace chess