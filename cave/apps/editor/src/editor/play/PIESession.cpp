#include "PIESession.h"

#include "cave/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "engine/private/core/diagnostics/DebugIdAllocator.h"
#include "engine/private/runtime/framework/IScriptService.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

namespace cave {

PIESession::PIESession(IApplication& p_app)
    : m_app(p_app)
    , m_debug_id(MakeDebugId(this)) {
}

bool PIESession::EnsureGameModuleLoaded() {
    if (m_game) {
        return true;
    }

    if (!m_game_handle.LoadFromDll(m_desc.game_dll.c_str())) {
        return false;
    }

    m_game = m_game_handle.Get();
    return m_game != nullptr;
}

bool PIESession::Start(const PIEStartDesc& p_desc) {
    DEV_ASSERT(m_running == false);

    m_desc = p_desc;

    if (!EnsureGameModuleLoaded()) return false;

    SceneRegistry* reg = m_app.GetSceneRegistry();
    if (!reg) return false;

    Scene* scene = reg->Resolve(p_desc.edit_scene);
    if (!scene) return false;

    PIEHostServices host(m_app, *scene, {});

    m_game->OnModuleLoaded(host);
    host.FlushSceneCommands();
    return true;
}

void PIESession::Stop() {
    if (m_running) {
        OnSimEnd();
        m_running = false;
    }

    m_game_handle.Unload();
    m_game = nullptr;
}

void PIESession::OnSimBegin(SceneId p_scene_id, ViewId p_view_id) {
    SceneRegistry& scene_manager = *m_app.GetSceneRegistry();

    m_pie_scene = scene_manager.Clone(p_scene_id);

    Scene* scene = scene_manager.Resolve(m_pie_scene);
    DEV_ASSERT(scene);
    m_app.ScriptService()->OnSimBegin(*scene);

    m_app.GetSceneScheduler().Register(this);

    m_host = std::make_unique<PIEHostServices>(m_app, *scene, p_view_id);
    m_game->OnGameBegin(*m_host);
    m_host->FlushSceneCommands();

    m_running = true;
}

void PIESession::OnSimEnd() {
    SceneRegistry& scene_manager = *m_app.GetSceneRegistry();

    m_running = false;

    if (Scene* scene = m_app.GetSceneRegistry()->Resolve(m_pie_scene)) {
        m_app.ScriptService()->OnSimEnd();

        m_game->OnGameEnd(*m_host);
    }

    m_app.GetSceneScheduler().Unregister(this);

    scene_manager.Destroy(m_pie_scene);
    m_pie_scene = {};
}

void PIESession::CollectSceneTicks(std::vector<SceneTickRequest>& p_out) {
    p_out.push_back({ SceneTickMode::Simulation, m_pie_scene });
}

void PIESession::Tick(const FrameTime& p_time) {
    if (!m_running || !m_game) {
        return;
    }

    SceneRegistry* reg = m_app.GetSceneRegistry();
    Scene* scene = reg->Resolve(m_pie_scene);
    if (!scene) return;

    m_game->Tick(*m_host, p_time);
    m_host->FlushSceneCommands();
}

void PIESession::BuildPIESceneFromEdit(Scene& p_edit, Scene& p_pie) {
    (void)p_edit;
    (void)p_pie;
}

}  // namespace cave
