#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/core/time/FrameTime.h"
#include "cave/game/GameModuleHandle.h"
#include "cave/runtime/framework/EngineServices.h"

#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/play/PIEHostServices.h"

namespace cave {

class IGameModule;
class Scene;

struct PIEStartDesc {
    std::string game_dll;
    std::string game_id;
    SceneId edit_scene;
};

class PIESession : public NonCopyable,
                   public ISceneOwner {
public:
    explicit PIESession(EngineServices& services);

    bool start(PIEStartDesc desc);
    void stop();

    void onSimBegin(SceneId scene_id, ViewId view_id);
    void onSimEnd();

    void tick(const FrameTime& time);

    bool running() const { return running_; }
    SceneId getPIESceneId() const { return pie_scene_; }

    void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) override;
    void commitSceneChange() override;

    DebugId debugId() const override { return debug_id_; }

private:
    bool ensureGameModuleLoaded();

    EngineServices& services_;
    const DebugId debug_id_;

    bool running_{ false };

    PIEStartDesc start_desc_{};

    GameModuleHandle game_module_handle_;
    IGameModule* game_module_{ nullptr };

    SceneId pie_scene_{};
    std::unique_ptr<PIEHostServices> host_;
};

}  // namespace cave
