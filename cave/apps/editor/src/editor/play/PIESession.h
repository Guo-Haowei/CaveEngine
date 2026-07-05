#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/core/time/FrameTime.h"
#include "cave/game/GameModuleHandle.h"
#include "cave/runtime/framework/EngineServices.h"

// @TODO: refactor
#include "engine/private/runtime/scene/SceneOwner.h"

namespace cave {

class IGameModule;
class Scene;

struct PIEStartDesc {
    std::string game_dll;
    std::string game_id;
};

class PIESession : public NonCopyable,
                   public SceneOwner {
public:
    explicit PIESession(EngineServices& services);

    bool start(PIEStartDesc desc);
    void stop();

    void beginPIESession(SceneId scene_id, ViewId view_id);
    void endPIESession();

    void tick(const FrameTime& time);

    bool running() const { return m_running; }
    SceneId getPIESceneId() const { return m_pie_scene; }

    void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    bool ensureGameModuleLoaded();

    SceneContext makeSceneContext(Scene& scene);
    Scene* beginPIEScene(Scene* asset_scene);
    void endPIEScene();

    void commitSceneChange(std::string&& path) override;
    void commitSceneReload() override {}

    EngineServices& m_engine_services;
    const DebugId m_debug_id;

    bool m_running{ false };

    PIEStartDesc m_start_desc{};

    GameModuleHandle m_game_module_handle;
    IGameModule* m_game_module{};

    SceneId m_pie_scene{};
};

}  // namespace cave
