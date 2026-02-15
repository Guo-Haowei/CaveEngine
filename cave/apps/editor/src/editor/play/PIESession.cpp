#include "PIESession.h"

#include "cave/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneMutator.h"
#include "engine/private/core/diagnostics/DebugIdAllocator.h"
#include "engine/private/runtime/framework/IScriptService.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"
#include "editor/play/PIEHostServices.h"

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
    if (m_running) {
        Stop();
    }

    m_desc = p_desc;

    if (!EnsureGameModuleLoaded()) {
        return false;
    }

    SceneRegistry* reg = m_app.GetSceneRegistry();
    if (!reg) {
        return false;
    }

    Scene* scene = reg->Resolve(p_desc.edit_scene);
    if (!scene) {
        return false;
    }

    PIEHostServices host(m_app, p_desc.edit_scene);
    GameInitDesc desc{
        .mode = AppMode::Editor,
        .game_id = "MyGame",
    };

    SceneCommandWriter cb(*m_app.GetAssetRegistry());
    //m_game->OnSceneBegin(*scene, host, desc, cb);
    if (!cb.Empty()) {
        SceneMutator mut(*scene);
        cb.Playback(mut);
    }
    return true;
}

void PIESession::Stop() {
    if (!m_running) {
        return;
    }

    OnSimEnd();

#if 0
    SceneRegistry* reg = m_app.GetSceneRegistry();
    Scene* pie_scene = reg ? reg->GetScene(m_pie_scene_id) : nullptr;

    if (m_game && pie_scene) {
        PIEHostServices pie_host(m_app, m_desc.game_view, m_pie_scene_id);
        m_game->ShutdownWorld(*pie_scene, pie_host);
    }

    // Destroy PIE scene
    if (reg && m_pie_scene_id != 0) {
        reg->DestroyScene(m_pie_scene_id);  // adjust to your API
    }

    m_pie_scene_id = 0;
    m_running = false;

    // Option A: keep DLL loaded for faster Play
    // Option B: unload on stop:
    // m_game_handle.Unload(); m_game = nullptr; m_registered = false;
#endif
}

void PIESession::OnSimBegin(SceneId p_scene_id) {
    SceneRegistry& scene_manager = *m_app.GetSceneRegistry();

    m_scene_id = scene_manager.Clone(p_scene_id);

    Scene* scene = scene_manager.Resolve(m_scene_id);
    DEV_ASSERT(scene);
    m_app.ScriptService()->OnSimBegin(*scene);

    m_app.GetSceneScheduler().Register(this);

    m_running = true;
}

void PIESession::OnSimEnd() {
    SceneRegistry& scene_manager = *m_app.GetSceneRegistry();

    m_running = false;

    if (Scene* scene = m_app.GetSceneRegistry()->Resolve(m_scene_id)) {
        m_app.ScriptService()->OnSimEnd();
    }

    m_app.GetSceneScheduler().Unregister(this);

    scene_manager.Destroy(m_scene_id);
    m_scene_id = {};
}

void PIESession::CollectSceneTicks(std::vector<SceneTickRequest>& p_out) {
    p_out.push_back({ SceneTickMode::Simulation, m_scene_id });
}

void PIESession::Tick(const FrameTime& p_time) {
    if (!m_running || !m_game) {
        return;
    }

    unused(p_time);

#if 0
    SceneRegistry* reg = m_app.GetSceneRegistry();
    Scene* pie_scene = reg ? reg->GetScene(m_pie_scene_id) : nullptr;
    if (!pie_scene)
        return;

    PIEHostServices pie_host(m_app, m_desc.game_view, m_pie_scene_id);

    GameTime t{};
    t.dt = p_dt;
    t.frame_index = 0;  // you can pass app frame index if you have it

    m_game->Tick(*pie_scene, pie_host, t);
#endif
}

void PIESession::BuildPIESceneFromEdit(Scene& p_edit, Scene& p_pie) {
    (void)p_edit;
    (void)p_pie;
}

}  // namespace cave
