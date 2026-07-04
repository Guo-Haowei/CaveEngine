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
    virtual void commitSceneChange() = 0;
    virtual void commitSceneReload() = 0;

    void requestSceneChange(std::string scene) override {
        pending_change_ = Some(std::move(scene));
    }

    void requestReload() {
        pending_reload_ = true;
    }

protected:
    Option<std::string> pending_change_;
    bool pending_reload_;
};

}  // namespace cave
