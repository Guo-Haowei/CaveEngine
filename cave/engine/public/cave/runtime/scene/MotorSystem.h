// =============================================================================
// File: cave/runtime/scene/MotorSystem.h
// =============================================================================
#pragma once
#include <memory>

#include "cave/core/math/Box.h"
#include "cave/core/math/Vec.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

struct ContactComponent;
struct MotorComponent;
struct VelocityComponent;
class TileWorldSystem;
class TransformComponent;
class ColliderComponent;
class SceneQuery;

math::Box2 ComputeWorldAABB(const TransformComponent& transform,
                            const ColliderComponent& collider);

class CollisionSystem;

class MotorSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::Motor)

public:
    MotorSystem();
    ~MotorSystem();

private:
    void start(SceneContext&) override {}
    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return m_debug_id; }

    SceneTickDomain domain() const override { return SceneTickDomain::Simulate; }

    void moveKinematic2D(const TileWorldSystem& tile_world,
                         TransformComponent& transform,
                         VelocityComponent& vel,
                         const ColliderComponent& collider,
                         const MotorComponent& motor,
                         ContactComponent* contact,
                         math::Vec2f desired_delta);

    void runTileWorldCollision(SceneTickContext& ctx);

    const DebugId m_debug_id;

    std::unique_ptr<CollisionSystem> m_collision;
};

}  // namespace cave
