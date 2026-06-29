#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/scene/SceneTickContext.h"

namespace cave {

struct FrameTime;
class SceneRegistry;

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
    SceneScheduler(EngineServices& services) noexcept
        : services_(services) {
    }

    bool add(ISceneTickContributor* contributor);
    bool remove(ISceneTickContributor* contributor);

    void tick(const FrameTime& time);

private:
    EngineServices& services_;

    std::vector<ISceneTickContributor*> contributors_;
};

}  // namespace cave
