#pragma once
#include "cave/runtime/scene/ISceneOwner.h"

namespace cave {

struct FrameTime;
class SceneRegistry;

class SceneScheduler {
public:
    SceneScheduler(EngineServices& services) noexcept
        : services_(services) {
    }

    bool add(ISceneOwner* owner);
    bool remove(ISceneOwner* owner);

    void flushSceneCommands();
    void tick(const FrameTime& time);

private:
    EngineServices& services_;

    std::vector<ISceneOwner*> owners_;
};

}  // namespace cave
