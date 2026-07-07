#include "PIESession.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/string/StringUtils.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneContext.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

PIESession::PIESession(EngineServices& services)
    : m_engine_services(services)
    , m_debug_id(MakeDebugId(this)) {
}

PIESession::~PIESession() {
    endPIESession();
}

SceneContext PIESession::makeSceneContext(Scene& scene) {
    return SceneContext{
        .scene = scene,
        .query = SceneQuery(scene),
        .services = m_engine_services,
        .view_id = {},
        .scene_transition = this,
    };
}

void PIESession::beginPIEScene(SceneDesc&& desc, const Scene& asset_scene) {
    DEV_ASSERT(!m_pie_scene.valid());
    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    m_pie_scene = scene_reg.cloneScene(std::move(desc), asset_scene);

    Scene* scene = scene_reg.resolve(m_pie_scene);
    if (DEV_VERIFY(scene)) {
        scene->begin(SceneTickContext{
            .domain = SceneTickDomain::Simulate,
            .dt = 0.0f,
            .scene_ctx = makeSceneContext(*scene),
        });
    }
}

void PIESession::endPIEScene() {
    DEV_ASSERT(m_pie_scene.valid());

    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    Scene* scene = scene_reg.resolve(m_pie_scene);

    if (DEV_VERIFY(scene)) {
        scene->end();
        scene_reg.destroyScene(m_pie_scene);
    }

    m_pie_scene = {};
}

void PIESession::beginPIESession(const Guid& guid, ViewId view_id) {
    m_engine_services.sceneScheduler().add(this);
    m_view_id = view_id;

    auto& asset_reg = m_engine_services.assetRegistry();
    if (auto handle = asset_reg.findByGuid<Scene>(guid)) {
        if (const Scene* asset_scene = handle.unwrap_unchecked().get()) {
            beginPIEScene(
                {
                    .source = SceneSource::Runtime,
                    .debug_name = handle.unwrap_unchecked().meta()->name,
                },
                *asset_scene);
            return;
        }
    }

    LOG_ERROR(LogChannel::Asset, "failed to start PIE scene {}", guid.toString());
}

void PIESession::endPIESession() {
    m_engine_services.sceneScheduler().remove(this);
    m_view_id = {};

    if (m_pie_scene.valid()) {
        endPIEScene();
    }
}

void PIESession::commitSceneChange(std::string&& path) {
    DEV_ASSERT(!path.empty());

    endPIEScene();

    auto handle = m_engine_services.assetRegistry().findByPath<Scene>(path);
    if (handle.is_none()) {
        LOG_ERROR(LogChannel::Asset, "Failed to find asset '{}'", path);
        return;
    }

    Scene* asset_scene = handle.unwrap_unchecked().get();
    if (!asset_scene) {
        LOG_ERROR(LogChannel::Asset, "Failed to load asset '{}'", path);
        return;
    }

    beginPIEScene(
        {
            .source = SceneSource::Runtime,
            .debug_name = std::string(StringUtils::fileName(path)),
        },
        *asset_scene);
}

void PIESession::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    out_requests.push_back({ SceneTickDomain::Simulate,
                             m_pie_scene,
                             m_view_id,
                             *this });
}

void PIESession::tick(const FrameTime&) {
}

}  // namespace cave
