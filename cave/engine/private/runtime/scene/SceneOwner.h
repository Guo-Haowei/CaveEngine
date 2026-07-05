#pragma once
#include "cave/runtime/scene/ISceneTransitionRequests.h"

namespace cave {

class SceneOwner;

struct SceneTickRequest {
    SceneTickMode mode;
    SceneId scene_id;
    SceneOwner& owner;
};

class SceneOwner : public ISceneTransitionRequests {
public:
    virtual ~SceneOwner() = default;

    virtual void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) = 0;

    void requestSceneChange(std::string path) override;
    void requestSceneReload();

    void flushSceneCommands();

protected:
    virtual void commitSceneChange(std::string&& path) = 0;
    virtual void commitSceneReload() = 0;

private:
    Option<std::string> m_pending_change;
    bool m_pending_reload = false;
};

}  // namespace cave
