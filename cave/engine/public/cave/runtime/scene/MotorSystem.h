// =============================================================================
// File: cave/runtime/scene/MotorSystem.h
// =============================================================================
#pragma once
#include <unordered_set>

#include "cave/core/math/Box.h"
#include "cave/core/math/Vector.h"
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

struct EntityPair {
    ecs::Entity a;
    ecs::Entity b;

    static EntityPair make(ecs::Entity x, ecs::Entity y) {
        if (x > y) {
            std::swap(x, y);
        }
        return { x, y };
    }
};

struct EntityPairHash {
    std::size_t operator()(const EntityPair& p) const noexcept {
        uint32_t a = p.a.GetId();
        uint32_t b = p.b.GetId();

        uint64_t packed = (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
        return std::hash<uint64_t>{}(packed);
    }
};

struct EntityPairEqual {
    bool operator()(const EntityPair& lhs, const EntityPair& rhs) const noexcept {
        return lhs.a == rhs.a && lhs.b == rhs.b;
    }
};

class MotorSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::Motor)

    using TriggerCache = std::unordered_set<EntityPair, EntityPairHash, EntityPairEqual>;

public:
    MotorSystem();

    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return debug_id_; }

private:
    void moveKinematic2D(const TileWorldSystem& tile_world,
                         TransformComponent& transform,
                         VelocityComponent& vel,
                         const ColliderComponent& collider,
                         const MotorComponent& motor,
                         ContactComponent* contact,
                         math::Vec2f desired_delta);

    void runTileWorldCollision(SceneTickContext& ctx);
    void runCollisionPair(SceneTickContext& ctx);

    const DebugId debug_id_;

    TriggerCache trigger_cache_;
};

}  // namespace cave
