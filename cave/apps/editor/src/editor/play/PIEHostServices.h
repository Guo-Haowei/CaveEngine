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
    explicit PIEHostServices(IApplication& p_app,
                             Scene& p_scene,
                             ViewId p_view_id) noexcept;

    cave::AssetRegistry& AssetRegistry() override;
    cave::ecs::ComponentRegistry& ComponentRegistry() override;
    IntentDispatcher& Intent() override;
    const IGameInput& Input() const override;
    IUIRuntime& UI() override;
    LogWrapper& Log() override { return m_logger; }
    cave::SceneQuery& SceneQuery() override { return m_query; }
    SceneCommandWriter& SceneWriter() override { return m_writer; }

    ViewId GetViewId() const override { return m_view_id; }

    void FlushSceneCommands();

private:
    IApplication& m_app;
    LogWrapper m_logger;
    Scene& m_scene;
    ViewId m_view_id;
    cave::SceneQuery m_query;
    SceneCommandWriter m_writer;
};

}  // namespace cave
