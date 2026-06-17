#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

struct FrameTime;
class SceneRegistry;

enum class SceneTickMode {
    Editor,
    Simulation,
};

struct SceneTickRequest {
    SceneTickMode mode;
    SceneId scene_id;
};

class ISceneTickContributor {
public:
    virtual ~ISceneTickContributor() = default;

    virtual void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) = 0;

    virtual DebugId debugId() const = 0;
};

class SceneScheduler {
public:
    SceneScheduler(SceneRegistry& scene_manager) noexcept
        : scene_manager_(scene_manager) {
    }

    bool add(ISceneTickContributor* contributor);
    bool remove(ISceneTickContributor* contributor);

    void tick(const FrameTime& time);

private:
    SceneRegistry& scene_manager_;

    std::vector<ISceneTickContributor*> contributors_;
};

}  // namespace cave
