#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/EngineServices.h"

// @TODO: refactor
#include "engine/private/runtime/scene/SceneOwner.h"

namespace cave {

class IGameModule;
class GameSession;
class Guid;
class Scene;
struct SceneDesc;

class PIESession : public NonCopyable,
                   public SceneOwner {
public:
    explicit PIESession(EngineServices& services);
    ~PIESession();

    bool beginPIESession(const Guid& scene_guid, ViewId view_id);
    bool endPIESession();

    void tick(const FrameTime& time);

    bool running() const { return m_pie_scene.valid(); }
    SceneId getPIESceneId() const { return m_pie_scene; }

    void collectSceneTicks(Vector<SceneTickRequest>& out_requests) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    void beginPIEScene(SceneDesc&& desc, const Scene& asset_scene);
    void endPIEScene();

    void commitSceneChange(String&& path) override;
    void commitSceneReload() override {}

    EngineServices& m_engine_services;
    const DebugId m_debug_id;

    SceneId m_pie_scene{};
    ViewId m_view_id;

    Owner<GameSession> m_session;
};

}  // namespace cave
