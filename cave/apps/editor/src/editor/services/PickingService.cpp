#include "PickingService.h"

#include "cave/core/math/Ray.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

#include "editor/EditorState.h"
#include "editor/services/SelectionService.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector2f;
using math::Vector3f;
using math::Vector4f;

PickingService::PickingService(EditorState& p_editor) noexcept
    : m_editor(p_editor) {
}

void PickingService::Submit(PickRequest p_req) {
    m_request = Some(std::move(p_req));
}

void PickingService::Raycast(const PickData& data, const Scene& p_scene) {
    Vector2f ndc = (data.cursor / data.extent) * 2.0f - 1.0f;
    ndc.y = -ndc.y;
    Vector4f clip_near{ ndc, 0.0f, 1.0f };
    Vector4f clip_far{ ndc, 1.0f, 1.0f };

    const Matrix4x4f inv_pv = glm::inverse(data.proj_view);

    Vector4f world_near = inv_pv * clip_near;
    Vector4f world_far = inv_pv * clip_far;
    world_near /= world_near.w;
    world_far /= world_far.w;

    math::Ray ray(world_near.xyz, world_far.xyz);

    auto result = p_scene.Intersects(ray);
    SelectionKey key{
        .kind = SelectionKind::Entity,
        .doc = data.doc_id,
        .scene = data.scene_id,
        .entity = result.entity,
    };
    m_editor.SelectionService().Set(data.doc_id, key);
}

void PickingService::Tick() {
    auto [win_x, win_y] = m_editor.GetApp().GetDisplayManager()->GetWindowPos();

    if (m_request.is_none()) {
        return;
    }

    const Vector2f pos_screen = m_request.unwrap_unchecked().cursor + Vector2f(win_x, win_y);

    for (IPickConsumer* p : m_consumers) {
        DEV_ASSERT(p);
        if (!p) continue;
        auto opt = p->GetPickData(pos_screen);
        if (opt.is_none()) continue;

        PickData data = opt.unwrap_unchecked();

        Scene* scene = m_editor.GetApp().GetSceneRegistry()->Resolve(data.scene_id);
        if (!scene) continue;

        Raycast(data, *scene);

        // @TODO: this doesn't work with overlay
        break;
    }

    m_request = None();
}

void PickingService::Register(IPickConsumer* p_consumer) {
    DEV_ASSERT(p_consumer);
    if (!p_consumer) return;

    auto it = std::ranges::find(m_consumers, p_consumer);
    if (it != m_consumers.end()) return;

    m_consumers.push_back(p_consumer);

#if USING(USE_LOG)
    DebugId id = p_consumer->GetDebugId();
    LOG_VERBOSE("PickingService::Register: register picking consumer '{}(id:{})'", id.type, id.uid);
#endif
}

void PickingService::Unregister(IPickConsumer* p_consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), p_consumer),
        m_consumers.end());

#if USING(USE_LOG)
    DebugId id = p_consumer->GetDebugId();
    LOG_VERBOSE("PickingService::Unegister: unregister picking consumer '{}(id:{})'", id.type, id.uid);
#endif
}

}  // namespace cave
