#pragma once
#include "SceneOwner.h"

namespace cave {

struct FrameTime;
class SceneRegistry;

class SceneScheduler {
public:
    SceneScheduler(EngineServices& services) noexcept
        : services_(services) {
    }

    bool add(SceneOwner* owner);
    bool remove(SceneOwner* owner);

    void flushSceneCommands();
    void tick(const FrameTime& time);

private:
    EngineServices& services_;

    std::vector<SceneOwner*> owners_;
};

}  // namespace cave
