#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

struct ContactComponent;
struct MotorComponent;
struct VelocityComponent;
class TileWorldSystem;
class TransformComponent;
class ColliderComponent;

class MotorSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::Motor)

public:
    MotorSystem();

    void update(float dt) override;

    DebugId debugId() const override { return debug_id_; }

private:
    void moveKinematic2D(const TileWorldSystem& tile_world,
                         TransformComponent& transform,
                         VelocityComponent& vel,
                         const ColliderComponent& collider,
                         const MotorComponent& motor,
                         ContactComponent* contact,
                         math::Vec2f desired_delta);

    const DebugId debug_id_;
};

}  // namespace cave
