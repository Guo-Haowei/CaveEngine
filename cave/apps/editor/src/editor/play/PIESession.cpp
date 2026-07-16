#include "PIESession.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/game/GameModuleHandle.h"
#include "cave/runtime/game/GameSession.h"
#include "cave/runtime/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneRuntime.h"

#include "engine/private/runtime/assets/SceneAsset.h"
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

void PIESession::beginPIEScene(SceneDesc&& desc, const Scene& asset_scene) {
    DEV_ASSERT(!m_pie_scene.valid());
    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    m_pie_scene = scene_reg.cloneScene(std::move(desc), asset_scene);

    Scene* scene = scene_reg.resolve(m_pie_scene);
    if (DEV_VERIFY(scene)) {
        scene->begin(MakeOwner<SceneRuntime>(
            SceneTickDomain::Simulate,
            m_engine_services,
            *scene,
            m_view_id,
            m_session.get(),
            this));
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

bool PIESession::beginPIESession(const Guid& guid, ViewId view_id) {
    m_engine_services.sceneScheduler().add(this);
    m_view_id = view_id;

    auto& asset_reg = m_engine_services.assetRegistry();
    if (auto handle_opt = asset_reg.findByGuid<SceneAsset>(guid)) {
        auto handle = handle_opt.unwrap_unchecked();
        if (const SceneAsset* asset = handle.get()) {
            m_session = MakeOwner<GameSession>();

            IGameModule* game_module = m_engine_services.gameModule().get();
            if (DEV_VERIFY(game_module)) {
                game_module->startSession(*m_session);
            }

            beginPIEScene({ SceneSource::Runtime, handle.meta()->name }, asset->scene());
            return true;
        }
    }

    return false;
}

bool PIESession::endPIESession() {
    if (m_session) {
        IGameModule* game_module = m_engine_services.gameModule().get();
        if (DEV_VERIFY(game_module)) {
            game_module->endSession(*m_session);
        }

        m_session.reset();
    }

    m_engine_services.sceneScheduler().remove(this);
    m_view_id = {};

    if (m_pie_scene.valid()) {
        endPIEScene();
    }
    return true;
}

void PIESession::commitSceneChange(std::string&& path) {
    DEV_ASSERT(!path.empty());

    endPIEScene();

    auto handle_opt = m_engine_services.assetRegistry().findByPath<SceneAsset>(path);
    if (handle_opt.is_none()) {
        LOG_ERROR(LogChannel::Asset, "Failed to find asset '{}'", path);
        return;
    }

    SceneAsset* asset = handle_opt.unwrap_unchecked().get();
    if (!asset) {
        LOG_ERROR(LogChannel::Asset, "Failed to load asset '{}'", path);
        return;
    }

    beginPIEScene(
        {
            .source = SceneSource::Runtime,
            .debug_name = String(StringUtils::fileName(path)),
        },
        asset->scene());
}

void PIESession::collectSceneTicks(Vector<SceneTickRequest>& out_requests) {
    out_requests.push_back({ SceneTickDomain::Simulate,
                             m_pie_scene,
                             m_view_id,
                             *this });
}

void PIESession::tick(const FrameTime&) {
}

}  // namespace cave
