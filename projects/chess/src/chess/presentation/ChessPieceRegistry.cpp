#include "ChessPieceRegistry.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

void ChessPieceRegistry::initPool() {
    auto& query = host_.sceneQuery();

    auto add_piece = [&](Piece type, std::string_view name) {
        const uint8_t idx = std::to_underlying(type);
        for (int i = 1;; ++i) {
            Entity id = query.findFirstByName(std::format("{}_{}", name, i));
            if (!id.IsValid()) {
                break;
            }
            piece_pool_[idx].push_back({ id, true });
        }
    };

    add_piece(Piece::WP, "white_pawn");
    add_piece(Piece::WN, "white_knight");
    add_piece(Piece::WB, "white_bishop");
    add_piece(Piece::WR, "white_rook");
    add_piece(Piece::WQ, "white_queen");
    add_piece(Piece::WK, "white_king");
    add_piece(Piece::BP, "black_pawn");
    add_piece(Piece::BN, "black_knight");
    add_piece(Piece::BB, "black_bishop");
    add_piece(Piece::BR, "black_rook");
    add_piece(Piece::BQ, "black_queen");
    add_piece(Piece::BK, "black_king");
}

ecs::Entity ChessPieceRegistry::allocate(core::Piece piece) {
    auto& pool = piece_pool_[std::to_underlying(piece)];

    for (Entry& e : pool) {
        if (e.free) {
            e.free = false;
            return e.id;
        }
    }

    assert(0 && "run out of free piece!");
    return Entity::Null();
}

void ChessPieceRegistry::freeAll() {
    for (auto& pool : piece_pool_) {
        for (Entry& e : pool) {
            e.free = true;
        }
    }
}

}  // namespace chess
