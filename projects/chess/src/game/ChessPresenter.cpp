#include "ChessPresenter.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using cave::StringId;
using cave::math::Vector3f;

constexpr StringId kTranslationId = StringId("translation");

static Vector3f SquareToPosition(const core::Square& p_sq) {
    auto [file, rank] = p_sq.FileRank();

    return Vector3f((float)rank, 0.0f, (float)file);
}

void ChessPresenter::OnGameBegin(cave::SceneQuery& p_query) {
    m_selector = p_query.FindFirstEntity("grid_selector");
}

void ChessPresenter::OnGameEnd() {
    m_selector = Entity::Null();
}

void ChessPresenter::Present(const PresentationContext& p_ctx) {
    cave::SceneQuery& query = p_ctx.host.SceneQuery();

    auto& writer = p_ctx.host.SceneWriter();

    Vector3f position = SquareToPosition(p_ctx.selected);
    writer.SetProperty(m_selector,
                       cave::TransformComponent_Id,
                       kTranslationId,
                       position);
}

void ChessPresenter::HighlightSquare(core::Square p_sq,
                                     HighlightHint p_hint) {
    (void)p_sq;

    if (p_hint == HighlightHint::LegalMove) {
        return;
    }
}

}  // namespace chess
