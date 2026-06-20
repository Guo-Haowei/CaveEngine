#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/view/ViewQuery.h"

namespace cave {

class Scene;

class PIEHostServices final : public IHostServices {
public:
    explicit PIEHostServices(EngineServices& services,
                             Scene& scene,
                             ViewId view_id) noexcept;

    AssetRegistry& assetRegistry() override;
    ecs::ComponentRegistry& componentRegistry() override;
    DisplayService& displayService() override;
    IDebugDrawService& debugDraw() override;
    IntentDispatcher& intentDispatcher() override;
    const IGameInput& gameInput() const override;
    IUIRuntime& ui() override;
    SceneCommandWriter& sceneWriter() override { return writer_; }

    ViewId viewId() const override { return view_id_; }

    SceneQuery& sceneQuery() override { return scene_query_; }
    const ViewQuery& viewQuery() const override { return view_query_; }

    void flushSceneCommands();

private:
    EngineServices& services_;
    Scene& scene_;
    ViewId view_id_;
    SceneCommandWriter writer_;

    SceneQuery scene_query_;
    ViewQuery view_query_;
};

}  // namespace cave
