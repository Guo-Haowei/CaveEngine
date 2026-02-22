#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include "core/Position.h"
#include "ChessPresenter.h"

// clang-format off
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

class ChessMatchAuthority;

class ChessGameClient {
public:
    ChessGameClient(ChessMatchAuthority& p_auth);

    void OnGameBegin(cave::IHostServices& p_host);

    void OnGameEnd(cave::IHostServices& p_host);

    void Tick(cave::IHostServices& p_host);

    std::span<const core::Move> LegalMovesFromSquare(core::Square p_sq);

    const core::Position& Pos() const { return m_replica; }

    ChessPresenter& Presenter() { return m_presenter; }

private:
    void OnPositionChange();

    void ResetBoard();

    ChessMatchAuthority& m_auth;
    ChessPresenter m_presenter;

    core::Position m_replica;  // replicated position of auth

    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;
};

}  // namespace chess