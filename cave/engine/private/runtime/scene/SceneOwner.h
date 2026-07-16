#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/scene/ISceneTransitionRequests.h"

namespace cave {

class SceneOwner;

struct SceneTickRequest {
    SceneTickDomain mode;
    SceneId scene_id;
    ViewId view_id;
    SceneOwner& owner;
};

class SceneOwner : public ISceneTransitionRequests {
public:
    virtual ~SceneOwner() = default;

    virtual void collectSceneTicks(Vector<SceneTickRequest>& out_requests) = 0;

    void requestSceneChange(String path) override;
    void requestSceneReload();

    void flushSceneCommands();

protected:
    virtual void commitSceneChange(String&& path) = 0;
    virtual void commitSceneReload() = 0;

private:
    Option<String> m_pending_change;
    bool m_pending_reload = false;
};

}  // namespace cave
