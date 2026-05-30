#pragma once
#include "cave/core/NonCopyable.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/core/time/FrameTime.h"
#include "cave/game/GameModuleHandle.h"

#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/play/PIEHostServices.h"

namespace cave {

class Scene;
class IApplication;
class IGameModule;

struct PIEStartDesc {
    std::string game_dll;
    std::string game_id;
    SceneId edit_scene;
};

class PIESession : public NonCopyable,
                   public ISceneTickContributor {
public:
    explicit PIESession(IApplication& p_app);

    bool Start(const PIEStartDesc& p_desc);
    void Stop();

    void OnSimBegin(SceneId p_scene_id, ViewId p_view_id);
    void OnSimEnd();

    void Tick(const FrameTime& p_time);

    bool IsRunning() const { return m_running; }
    SceneId GetPIESceneId() const { return m_pie_scene; }

    void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) override;
    DebugId GetDebugId() const override { return m_debug_id; }

private:
    bool EnsureGameModuleLoaded();
    void BuildPIESceneFromEdit(Scene& p_edit, Scene& p_pie);

    IApplication& m_app;
    const DebugId m_debug_id;

    bool m_running = false;

    PIEStartDesc m_desc{};

    GameModuleHandle m_game_handle;
    IGameModule* m_game = nullptr;

    SceneId m_pie_scene{};
    std::unique_ptr<PIEHostServices> m_host;
};

}  // namespace cave
