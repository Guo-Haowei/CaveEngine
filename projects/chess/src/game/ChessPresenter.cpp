#include "ChessPresenter.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using cave::StringId;
using cave::math::Vector3f;
using chess::core::Square;

constexpr StringId kTranslationId = StringId("translation");

static Vector3f SquareToPosition(const core::Square& p_sq) {
    auto [file, rank] = p_sq.FileRank();

    return Vector3f((float)rank, 0.0f, (float)file);
}

void ChessPresenter::OnGameBegin(cave::SceneQuery& p_query) {
    m_highlights = {};

    m_selector = p_query.FindFirstEntity("grid_selector");

    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).ToString();
        m_tiles[i] = p_query.FindFirstEntity(name);
    }
}

void ChessPresenter::OnGameEnd() {
    m_selector = Entity::Null();
    for (Entity& e : m_tiles) e = Entity::Null();
}

void ChessPresenter::Present(const PresentationContext& p_ctx) {
    cave::SceneQuery& query = p_ctx.host.SceneQuery();

    auto& writer = p_ctx.host.SceneWriter();

    Vector3f position = SquareToPosition(m_selected);
    writer.SetProperty(m_selector,
                       cave::TransformComponent_Id,
                       kTranslationId,
                       position);

    for (uint8_t i = 0; i < 64; ++i) {
        const bool vis = m_highlights.Test(Square(i));
        const Entity tile = m_tiles[i];
        writer.SetProperty(tile,
                           cave::MeshRendererComponent_Id,
                           StringId("visibility"),
                           vis);
    }
}

}  // namespace chess
