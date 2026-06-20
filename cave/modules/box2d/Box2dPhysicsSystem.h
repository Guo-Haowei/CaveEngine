#pragma once
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

class Box2dPhysicsSystem : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::Physics2D)

public:
    Box2dPhysicsSystem();

    void update(float dt) override;

    void onAttach() override;
    void onDetach() override;

    DebugId debugId() const override { return debug_id_; }

protected:
    Option<uint32_t> world_id_;

    const DebugId debug_id_;
};

}  // namespace cave
