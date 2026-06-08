#include "ChessGameModule.h"

#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/core/Bitboard.h"
#include "chess/core/Piece.h"
#include "chess/game/ChessGameMode.h"
#include "chess/presentation/ChessViewFactory.h"

namespace chess {

using namespace cave;
using namespace cave::literals;

using cave::ecs::Entity;
using cave::math::Vector2i;
using cave::math::Vector3f;
using cave::math::Vector4f;
using chess::core::Color;
using chess::core::Piece;
using chess::core::PieceType;
using chess::core::Square;

// @TODO: use FEN instead
static constexpr std::array<std::array<Piece, 8>, 8> kInitialBoard = { {
    { Piece::WR, Piece::WN, Piece::WB, Piece::WQ, Piece::WK, Piece::WB, Piece::WN, Piece::WR },
    { Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP },
    { Piece::BR, Piece::BN, Piece::BB, Piece::BQ, Piece::BK, Piece::BB, Piece::BN, Piece::BR },
} };

ChessGameModule::ChessGameModule() = default;
ChessGameModule::~ChessGameModule() = default;

void ChessGameModule::OnModuleLoaded(IHostServices& p_host) {
    p_host.log().Ok(LogChannel::Game, "ChessClient Loaded");

    // @TODO: move it to present layer
    SpawnObjects(p_host);
}

void ChessGameModule::OnModuleUnloaded(IHostServices& p_host) {
    unused(p_host);
}

void ChessGameModule::OnGameBegin(IHostServices& p_host) {
    m_game = std::make_unique<ChessGameMode>(p_host);
    m_game->OnEnter(p_host);
}

void ChessGameModule::OnGameEnd(IHostServices& p_host) {
    m_game->OnExit(p_host);
    m_game.reset();
}

void ChessGameModule::Tick(IHostServices& p_host, const FrameTime& p_time) {
    m_game->Tick(p_host, p_time);
}

// @TODO: extract it,
// because other than starting pos,
// it can be used for any pos, for example, for puzzle mode
void ChessGameModule::SpawnObjects(IHostServices& p_host) {
    using chess::Piece;
    using ecs::Entity;

    Entity offset_node = p_host.sceneQuery().findFirstByName("transform");
    DEV_ASSERT(offset_node.IsValid());

    SceneCommandWriter& writer = p_host.sceneWriter();
    writer.SetNoSave(true);

    Entity piece_parent = writer.CreateTransformObject("pieces");
    Entity tile_parent = writer.CreateTransformObject("tiles");

    writer.AttachChild(piece_parent, offset_node);
    writer.AttachChild(tile_parent, offset_node);

    std::array<int, core::kPieceMax> counter{ 0 };

    chess::ChessSpawner spawner(writer, piece_parent);
    for (uint8_t rank = 0; rank < 8; ++rank) {
        for (uint8_t file = 0; file < 8; ++file) {
            spawner.SpawnTile(file, rank, {
                                              Vector4f(0.0f, 1.0f, 0.0f, 0.5f),
                                              nullptr,
                                              false,
                                              tile_parent,
                                          });

            const Piece p = chess::kInitialBoard[rank][file];
            if (p == Piece::Null) continue;
            spawner.SpawnPiece(p, file, rank, ++counter[std::to_underlying(p)]);
        }
    }

    spawner.SpawnTile(0, 0, {
                                Vector4f(1.0f, 0.0f, 0.0f, 0.5f),
                                "grid_selector",
                                true,
                                offset_node,
                            });
}

}  // namespace chess
