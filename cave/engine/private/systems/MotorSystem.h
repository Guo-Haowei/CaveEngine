#pragma once
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

class MotorSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::Motor)

public:
    MotorSystem();

    void update(float dt) override;

    DebugId debugId() const override { return debug_id_; }

private:
    const DebugId debug_id_;
};

}  // namespace cave
