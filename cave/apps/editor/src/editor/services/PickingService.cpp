#include "PickingService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/framework/AppServices.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/runtime/scene/SceneQueryService.h"
#include "engine/private/runtime/scene/Scene.h"

#include "editor/EditorIntent.h"
#include "editor/EditorServices.h"
#include "editor/services/SelectionService.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector2f;
using math::Vector3f;
using math::Vector4f;

PickingService::PickingService(AppServices& app_services,
                               EditorServices& editor_services)
    : app_services_(app_services)
    , editor_services_(editor_services)
    , debug_id_(MakeDebugId(this)) {

    app_services_.intentDispatcher().addHandler<PickIntent>(this);
}

PickingService::~PickingService() {
    app_services_.intentDispatcher().removeHandler<PickIntent>(this);
}

void PickingService::pick(math::Vector2f point_win) {
    app_services_.intentDispatcher().queue<PickIntent>(point_win);
}

void PickingService::raycast(const PickData& pick_data) {
    auto ray = math::Ray::unproject(pick_data.proj_view, pick_data.cursor_ndc);

    auto result = app_services_.sceneQuery().raycast(pick_data.scene_id, ray, {});

    SelectionKey key{
        .kind = SelectionKind::Entity,
        .doc = pick_data.doc_id,
        .scene = pick_data.scene_id,
        .entity = result.entity,
    };

    editor_services_.selection().Set(pick_data.doc_id, key);
}

bool PickingService::handleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<PickIntent*>(&p_intent)) {
        const Vector2f pos_screen = intent->pointer() + app_services_.displayService().windowPos();

        for (IPickConsumer* p : m_consumers) {
            DEV_ASSERT(p);
            if (!p) continue;
            auto opt = p->getPickData(pos_screen);
            if (opt.is_none()) continue;

            PickData data = opt.unwrap_unchecked();
            raycast(data);

            // @TODO: this doesn't work with overlay
            break;
        }
        return true;
    }

    return false;
}

void PickingService::addConsumer(IPickConsumer* consumer) {
    DEV_ASSERT(consumer);
    if (!consumer) return;

    auto it = std::ranges::find(m_consumers, consumer);
    if (it != m_consumers.end()) return;

    m_consumers.push_back(consumer);

#if USING(USE_LOG)
    DebugId id = consumer->debugId();
    LOG_TRACE(LogChannel::Picking, "+{}#{}", id.type, id.uid);
#endif
}

void PickingService::removeConsumer(IPickConsumer* consumer) {
    m_consumers.erase(
        std::remove(m_consumers.begin(), m_consumers.end(), consumer),
        m_consumers.end());

#if USING(USE_LOG)
    DebugId id = consumer->debugId();
    LOG_TRACE(LogChannel::Picking, "-{}#{}", id.type, id.uid);
#endif
}

}  // namespace cave
