#pragma once
#include <array>

#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Position.h"

// clang-format off
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

class ChessPieceRegistry {
    using Entity = ::cave::ecs::Entity;

public:
    ChessPieceRegistry(cave::IHostServices& host) noexcept
        : host_(host) {
    }

    void initPool();

    Entity allocate(core::Piece piece);

    void freeAll();

private:
    struct Entry {
        Entity id;
        bool free;
    };

    cave::IHostServices& host_;

    std::array<std::vector<Entry>, core::kPieceMax> piece_pool_;
};

}  // namespace chess
