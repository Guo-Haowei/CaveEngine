#pragma once
#include "cave/runtime/scene/ISceneSystem.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

class MotorSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::Motor)

public:
    MotorSystem();

    void update(float dt) override;

    DebugId debugId() const override { return debug_id_; }

private:
    void moveKinematic2D(SceneQuery& query,
                         ecs::Entity ent,
                         TransformComponent& transform,
                         VelocityComponent& vel,
                         const MotorComponent& motor,
                         math::Vec2f desired_delta);

    const DebugId debug_id_;
};

}  // namespace cave
