#include "PIESession.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
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

bool PIESession::ensureGameModuleLoaded() {
    if (m_game_module) {
        return true;
    }

    if (!m_game_module_handle.loadFromDll(m_start_desc.game_dll.c_str(), m_engine_services.nativeScripts())) {
        return false;
    }

    m_game_module = m_game_module_handle.get();

    return m_game_module != nullptr;
}

bool PIESession::start(PIEStartDesc start_desc) {
    DEV_ASSERT(m_running == false);

    m_start_desc = std::move(start_desc);

    return ensureGameModuleLoaded();
}

void PIESession::stop() {
    if (m_running) {
        endPIESession();
        m_running = false;
    }

    m_game_module_handle.unload();
    m_game_module = nullptr;
}

SceneContext PIESession::makeSceneContext(Scene& scene) {
    return SceneContext{
        .native_scripts = m_engine_services.nativeScripts(),
        .scene = scene,
        .scene_transition = *this,
        .query = SceneQuery(scene),
        .engine_services = m_engine_services,
    };
}

Scene* PIESession::beginPIEScene(Scene* asset_scene) {
    DEV_ASSERT(asset_scene);
    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    m_pie_scene = scene_reg.cloneScene(*asset_scene);

    Scene* scene = scene_reg.resolve(m_pie_scene);
    if (DEV_VERIFY(scene)) {
        SceneContext ctx = makeSceneContext(*scene);
        scene->onSimBegin(ctx);
    }
    return scene;
}

void PIESession::endPIEScene() {
    if (!m_pie_scene.isValid()) {
        return;
    }

    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    Scene* scene = scene_reg.resolve(m_pie_scene);

    if (DEV_VERIFY(scene)) {
        SceneContext ctx = makeSceneContext(*scene);
        scene->onSimEnd(ctx);
        scene_reg.destroyScene(m_pie_scene);
    }

    m_pie_scene = {};
}

void PIESession::beginPIESession(SceneId scene_id, ViewId view_id) {
    m_engine_services.sceneScheduler().add(this);

    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    Scene* pie_scene = beginPIEScene(scene_reg.resolve(scene_id));
    if (DEV_VERIFY(pie_scene)) {
        if (m_game_module) {
            // @BUG: host still points to old scene, after scene change
            // this is dangerous, but it's fine for now
            // because we are going to remove PIEHostServices entirely.
            m_host = std::make_unique<PIEHostServices>(m_engine_services, *pie_scene, view_id);
            m_game_module->onGameBegin(*m_host);
            m_host->flushSceneCommands();
        }

        m_running = true;
    }
}

void PIESession::endPIESession() {
    m_engine_services.sceneScheduler().remove(this);

    if (!m_pie_scene.isValid()) {
        return;
    }

    m_running = false;

    endPIEScene();

    if (m_game_module && m_host) {
        m_game_module->onGameEnd(*m_host);
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

    beginPIEScene(asset_scene);
}

void PIESession::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    out_requests.push_back({ SceneTickMode::Simulation, m_pie_scene, *this });
}

void PIESession::tick(const FrameTime& time) {
    if (!m_running || !m_game_module) {
        return;
    }

    SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
    Scene* scene = scene_reg.resolve(m_pie_scene);
    if (!scene) return;

    m_game_module->tick(*m_host, time);
    m_host->flushSceneCommands();
}

}  // namespace cave
