#include "MotorSystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;
using ::cave::ecs::Entity;

namespace {
constexpr float kMinGroundSupport = 0.05f;  // tune; tile size is 1.0
constexpr float kStepOffset = 0.05f;

Box2 ComputeWorldAABB(const TransformComponent& transform,
                      const ColliderComponent& collider) {
    const Shape& shape = collider.shape();
    Vec2f translation = transform.GetTranslation().xy;

    return {
        translation - Vec2f(shape.data.half.xy),
        translation + Vec2f(shape.data.half.xy),
    };
}

cave::math::Box2 MoveBox(cave::math::Box2 box, cave::math::Vec2f delta) {
    box.SetMinMax(box.Min() + delta, box.Max() + delta);
    return box;
}

bool OverlapsSolidTiles(const Box2& aabb, const TileWorldSystem& world) {
    return !world.querySolidTiles(aabb).empty();
}

}  // namespace

MotorSystem::MotorSystem()
    : debug_id_(MakeDebugId(this)) {
}

void MotorSystem::update(float dt) {
    Scene& scene = context().scene;

    SceneQuery query(scene);
    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();

    auto view = scene.view<MotorComponent, VelocityComponent, ColliderComponent, TransformComponent>();

    for (auto [ent, motor, vel, collider, transform] : view) {
        Vec2f delta = vel.linear.xy;
        delta *= dt;

        Box2 body = ComputeWorldAABB(transform, collider);

        if (delta.x != 0.0f) {
            Box2 next_x = MoveBox(body, { delta.x, 0.0f });

            if (!OverlapsSolidTiles(next_x, *tile_world)) {
                transform.Translate({ delta.x, 0.0f, 0.0f });
                body = next_x;
            } else {
                vel.linear.x = 0.0f;
            }
        }

        if (delta.y != 0.0f) {
            Box2 next_y = MoveBox(body, { 0.0f, delta.y });

            if (!OverlapsSolidTiles(next_y, *tile_world)) {
                transform.Translate({ 0.0f, delta.y, 0.0f });
            } else {
                vel.linear.y = 0.0f;
            }
        }
    }
}

void MotorSystem::moveKinematic2D(SceneQuery& query,
                                  Entity ent,
                                  TransformComponent& transform,
                                  VelocityComponent& vel,
                                  const MotorComponent& motor,
                                  Vec2f desired_delta) {
    auto collider = query.component<ColliderComponent>(ent);
    DEV_ASSERT(collider);

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    Box2 body = ComputeWorldAABB(transform, *collider);

    Vec2f actual_delta{ 0.0f, 0.0f };

    if (desired_delta.x != 0.0f) {
        float dx = ResolveHorizontalMovement(body, desired_delta.x, *tile_world);

        if (dx != 0.0f) {
            transform.TranslateWorld({ dx, 0.0f, 0.0f });
            body = MoveBox(body, { dx, 0.0f });
        }

        actual_delta.x = dx;

        if (dx != desired_delta.x) {
            vel.linear.x = 0.0f;

            if (contact) {
                if (desired_delta.x < 0.0f) {
                    contact->hit_left = true;
                } else {
                    contact->hit_right = true;
                }
            }
        }
    }

    if (desired_delta.y > 0.0f) {
        auto result = ResolveUpMovement(body, desired_delta.y, *tile_world, motor.step_offset);

        if (result.dy != 0.0f) {
            transform.TranslateWorld({ 0.0f, result.dy, 0.0f });
            body = MoveBox(body, { 0.0f, result.dy });
        }

        actual_delta.y = result.dy;

        if (result.hit) {
            vel.linear.y = 0.0f;

            if (contact) {
                contact->hit_ceiling = true;
            }
        }
    } else if (desired_delta.y < 0.0f) {
        auto result = ResolveDownMovement(body, desired_delta.y, *tile_world, motor.min_ground_support);

        if (result.dy != 0.0f) {
            transform.TranslateWorld({ 0.0f, result.dy, 0.0f });
            body = MoveBox(body, { 0.0f, result.dy });
        }

        actual_delta.y = result.dy;

        if (result.hit) {
            vel.linear.y = 0.0f;

            if (contact) {
                contact->hit_floor = true;
                contact->grounded = true;
            }
        }
    }

    if (contact) {
        contact->actual_delta = actual_delta;
    }
}

}  // namespace cave
