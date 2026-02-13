#include "PIESession.h"

#include "cave/game/IGameModule.h"

#include "editor/play/PIEHostServices.h"

namespace cave {

PIESession::PIESession(IApplication& p_app)
    : m_app(p_app) {
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

    PIEHostServices host(m_app, {});

    if (!m_registered) {
        m_game->RegisterTypes(host);
        m_game->RegisterSystems(host);
        m_registered = true;
    }

    SceneRegistry* reg = m_app.GetSceneRegistry();
    if (!reg) {
        return false;
    }

#if 0
    // Create PIE scene
    m_pie_scene_id = reg->CreateScene(/*name*/ "PIE");  // adjust to your API
    Scene* pie_scene = reg->GetScene(m_pie_scene_id);
    Scene* edit_scene = reg->GetScene(m_desc.edit_scene);
    if (!pie_scene || !edit_scene)
        return false;

    // Rebuild host with actual pie scene id (if you want host to expose it)
    PIEHostServices pie_host(m_app, m_desc.game_view, m_pie_scene_id);

    // Copy edit -> pie (can be no-op for first implementation)
    BuildPIESceneFromEdit_(*edit_scene, *pie_scene);

    // Call game CreateWorld ONCE
    GameInitDesc init{};
    init.mode = AppMode::Editor;
    init.game_id = m_desc.game_id.c_str();
    init.map_or_level = m_desc.map_or_level.empty() ? nullptr : m_desc.map_or_level.c_str();

    m_game->CreateWorld(*pie_scene, pie_host, init);

    m_running = true;
#endif
    return true;
}

void PIESession::Stop() {
    if (!m_running)
        return;

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
