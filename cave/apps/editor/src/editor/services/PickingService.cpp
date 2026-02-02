#include "PickingService.h"

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "editor/EditorState.h"

namespace cave {

using math::Vector2f;

PickingService::PickingService(EditorState& p_editor) noexcept
    : m_editor(p_editor) {
}

void PickingService::Submit(PickRequest p_req) {
    m_requests.emplace_back(std::move(p_req));
}

void PickingService::Tick() {
    auto [win_x, win_y] = m_editor.GetApp().GetDisplayManager()->GetWindowPos();

    for (const PickRequest& req : m_requests) {
        Vector2f click_pos = req.cursor + Vector2f(win_x, win_y) - req.pos;
        Vector2f noramlized = click_pos / req.size;

        LOG("request tab: {}, x: {}, y: {}",
            req.tab_id.index,
            noramlized.x,
            noramlized.y);
    }

    m_requests.clear();
}

}  // namespace cave
