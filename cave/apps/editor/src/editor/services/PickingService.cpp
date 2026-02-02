#include "PickingService.h"

namespace cave {

PickingService::PickingService(EditorState& p_editor) noexcept
    : m_editor(p_editor) {
}

void PickingService::Submit(PickRequest p_req) {
    m_requests.emplace_back(std::move(p_req));
}

void PickingService::Tick() {
    for (const PickRequest& req : m_requests) {
        LOG("request tab: {}, x: {}, y: {}",
            req.tab_id.index,
            req.x_view_px,
            req.y_view_px);
    }

    m_requests.clear();
}

}  // namespace cave
