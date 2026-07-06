#include "PickingService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/math/Ray.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/intent/IntentBus.h"

#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

#include "editor/EditorIntent.h"
#include "editor/services/EditorServices.h"
#include "editor/services/SelectionService.h"

namespace cave {

using math::Mat4f;
using math::Vec2f;
using math::Vec3f;
using math::Vec4f;

PickingService::PickingService(EngineServices& app_services,
                               EditorServices& editor_services)
    : m_app_services(app_services)
    , m_editor_services(editor_services)
    , m_debug_id(MakeDebugId(this)) {

    m_app_services.intentBus().addHandler<PickIntent>(this);
}

PickingService::~PickingService() {
    m_app_services.intentBus().removeHandler<PickIntent>(this);
}

void PickingService::pick(math::Vec2f point_win) {
    m_app_services.intentBus().queue<PickIntent>(point_win);
}

void PickingService::raycast(const PickData& pick_data) {
    auto ray = math::Ray::unproject(pick_data.proj_view, pick_data.cursor_ndc);

    Scene* scene = m_app_services.sceneRegistry().resolve(pick_data.scene_id);
    if (!scene) {
        return;
    }

    SceneQuery query(*scene);

    auto result = query.raycast(ray, {});

    SelectionKey key{
        .kind = SelectionKind::Entity,
        .doc = pick_data.doc_id,
        .scene = pick_data.scene_id,
        .entity = result.entity,
    };

    m_editor_services.selection().Set(pick_data.doc_id, key);
}

bool PickingService::handleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<PickIntent*>(&p_intent)) {
        const Vec2f pos_screen = intent->pointer() + m_app_services.displayService().windowPos();

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
