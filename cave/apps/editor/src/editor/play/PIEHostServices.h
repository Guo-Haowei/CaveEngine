#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

namespace cave {

class Scene;

class PIEHostServices final : public IHostServices {
public:
    explicit PIEHostServices(IApplication& app,
                             Scene& scene,
                             ViewId view_id) noexcept;

    AssetRegistry& assetRegistry() override;
    ecs::ComponentRegistry& componentRegistry() override;
    IntentDispatcher& intentDispatcher() override;
    const IGameInput& gameInput() const override;
    IUIRuntime& ui() override;
    LogWrapper& log() override { return logger_; }
    SceneQuery& sceneQuery() override { return query_; }
    SceneCommandWriter& sceneWriter() override { return writer_; }

    ViewId viewId() const override { return view_id_; }

    void flushSceneCommands();

private:
    IApplication& app_;
    LogWrapper logger_;
    Scene& scene_;
    ViewId view_id_;
    cave::SceneQuery query_;
    SceneCommandWriter writer_;
};

}  // namespace cave
