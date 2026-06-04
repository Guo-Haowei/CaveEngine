#include "PickingService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/runtime/display/DisplayService.h"
#include "engine/private/runtime/scene/SceneQueryService.h"
#include "engine/private/runtime/scene/Scene.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"
#include "editor/services/SelectionService.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector2f;
using math::Vector3f;
using math::Vector4f;

PickingService::PickingService(EditorState& p_editor)
    : m_editor(p_editor)
    , m_debug_id(MakeDebugId(this)) {
    m_editor.GetApp().IntentDispatcher()->AddHandler<PickIntent>(this);
}

PickingService::~PickingService() {
    m_editor.GetApp().IntentDispatcher()->RemoveHandler<PickIntent>(this);
}

void PickingService::Pick(math::Vector2f p_point_win) {
    m_editor.GetApp().IntentDispatcher()->Queue<PickIntent>(p_point_win);
}

void PickingService::Raycast(const PickData& p_pick_data) {
    Vector4f clip_near{ p_pick_data.cursor_ndc, 0.0f, 1.0f };
    Vector4f clip_far{ p_pick_data.cursor_ndc, 1.0f, 1.0f };

    const Matrix4x4f inv_pv = glm::inverse(p_pick_data.proj_view);

    Vector4f world_near = inv_pv * clip_near;
    Vector4f world_far = inv_pv * clip_far;
    world_near /= world_near.w;
    world_far /= world_far.w;

    math::Ray ray(world_near.xyz, world_far.xyz);

    auto result = m_editor.GetApp().SceneQueryService().Raycast(p_pick_data.scene_id, ray, {});

    SelectionKey key{
        .kind = SelectionKind::Entity,
        .doc = p_pick_data.doc_id,
        .scene = p_pick_data.scene_id,
        .entity = result.entity,
    };
    m_editor.SelectionService().Set(p_pick_data.doc_id, key);
}

bool PickingService::HandleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<PickIntent*>(&p_intent)) {
        IApplication& app = m_editor.GetApp();
        auto [win_x, win_y] = app.GetDisplayService()->GetWindowPos();
        const Vector2f pos_screen = intent->pointer + Vector2f(win_x, win_y);

        for (IPickConsumer* p : m_consumers) {
            DEV_ASSERT(p);
            if (!p) continue;
            auto opt = p->GetPickData(pos_screen);
            if (opt.is_none()) continue;

            PickData data = opt.unwrap_unchecked();
            Raycast(data);

            // @TODO: this doesn't work with overlay
            break;
        }
        return true;
    }

    return false;
}

void PickingService::Register(IPickConsumer* p_consumer) {
    DEV_ASSERT(p_consumer);
    if (!p_consumer) return;

    auto it = std::ranges::find(m_consumers, p_consumer);
    if (it != m_consumers.end()) return;

    m_consumers.push_back(p_consumer);

#if USING(USE_LOG)
    DebugId id = p_consumer->GetDebugId();
    LOG_TRACE(LogChannel::Picking, "+{}#{}", id.type, id.uid);
#endif
}

void PickingService::Unregister(IPickConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());

#if USING(USE_LOG)
    DebugId id = p_consumer->GetDebugId();
    LOG_TRACE(LogChannel::Picking, "-{}#{}", id.type, id.uid);
#endif
}

}  // namespace cave
