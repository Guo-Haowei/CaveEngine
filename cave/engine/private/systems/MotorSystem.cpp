#include "MotorSystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneQuery.h"
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

bool Overlap1DStrict(float a_min, float a_max, float b_min, float b_max) {
    return a_max > b_min && a_min < b_max;
}

float OverlapAmount1D(float a_min, float a_max, float b_min, float b_max) {
    return math::min(a_max, b_max) - math::max(a_min, b_min);
}

float ResolveHorizontalMovement(const Box2& body,
                                float dx,
                                const TileWorldSystem& world) {
    if (dx == 0.0f) {
        return 0.0f;
    }

    Box2 query = MoveBox(body, { dx, 0.0f });
    query.UnionBox(body);

    float resolved_dx = dx;

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        if (!Overlap1DStrict(body.Min().y, body.Max().y,
                             solid.Min().y, solid.Max().y)) {
            continue;
        }

        if (dx > 0.0f) {
            if (body.Max().x <= solid.Min().x) {
                const float candidate_dx = solid.Min().x - body.Max().x;
                resolved_dx = std::min(resolved_dx, candidate_dx);
            }
        } else {
            if (body.Min().x >= solid.Max().x) {
                const float candidate_dx = solid.Max().x - body.Min().x;
                resolved_dx = std::max(resolved_dx, candidate_dx);
            }
        }
    }

    return resolved_dx;
}

struct VerticalMoveResult {
    bool hit = false;
    float dy = 0.0f;
};

VerticalMoveResult ResolveUpMovement(const Box2& body,
                                     float dy,
                                     const TileWorldSystem& world,
                                     float step_offset) {
    VerticalMoveResult result;
    result.dy = dy;

    if (dy <= 0.0f) {
        return result;
    }

    Box2 query = MoveBox(body, { 0.0f, dy });
    query.UnionBox(body);

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        const bool x_overlap =
            body.Max().x - step_offset >= solid.Min().x &&
            body.Min().x + step_offset <= solid.Max().x;

        if (!x_overlap) {
            continue;
        }

        if (body.Max().y <= solid.Min().y && body.Max().y + dy >= solid.Min().y) {
            result.hit = true;
            result.dy = std::min(result.dy, solid.Min().y - body.Max().y);
        }
    }

    return result;
}

VerticalMoveResult ResolveDownMovement(const Box2& body,
                                       float dy,
                                       const TileWorldSystem& world,
                                       float min_ground_support) {
    VerticalMoveResult result;
    result.dy = dy;

    if (dy >= 0.0f) {
        return result;
    }

    Box2 query = MoveBox(body, { 0.0f, dy });
    query.UnionBox(body);

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        const float overlap_x = OverlapAmount1D(
            body.Min().x,
            body.Max().x,
            solid.Min().x,
            solid.Max().x);

        if (overlap_x <= min_ground_support) {
            continue;
        }

        if (body.Min().y >= solid.Max().y && body.Min().y + dy <= solid.Max().y) {
            result.hit = true;
            result.dy = std::max(result.dy, solid.Max().y - body.Min().y);
        }
    }

    return result;
}

}  // namespace

MotorSystem::MotorSystem()
    : debug_id_(MakeDebugId(this)) {
}

void MotorSystem::update(float dt) {
    Scene& scene = context().scene;

    SceneQuery query(scene);
    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();
    DEV_ASSERT(tile_world);

    auto view = scene.view<MotorComponent, VelocityComponent, ColliderComponent, TransformComponent>();

    for (auto [ent, motor, vel, collider, transform] : view) {
        if (motor.affected_by_gravity) {
            vel.linear.y += motor.gravity * dt;
            vel.linear.y = math::max(vel.linear.y, motor.terminal_fall_speed);
        }

        Vec2f delta = vel.linear.xy;
        delta *= dt;

        ContactComponent* contact = query.component<ContactComponent>(ent);
        moveKinematic2D(*tile_world, transform, vel, collider, motor, contact, delta);
    }
}

void MotorSystem::moveKinematic2D(const TileWorldSystem& tile_world,
                                  TransformComponent& transform,
                                  VelocityComponent& vel,
                                  const ColliderComponent& collider,
                                  const MotorComponent& motor,
                                  ContactComponent* contact,
                                  Vec2f desired_delta) {
    Box2 body = ComputeWorldAABB(transform, collider);

    Vec2f actual_delta{ 0.0f, 0.0f };

    if (desired_delta.x != 0.0f) {
        float dx = ResolveHorizontalMovement(body, desired_delta.x, tile_world);

        if (dx != 0.0f) {
            transform.Translate({ dx, 0.0f, 0.0f });
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
        auto result = ResolveUpMovement(body,
                                        desired_delta.y,
                                        tile_world,
                                        motor.step_offset);

        if (result.dy != 0.0f) {
            transform.Translate({ 0.0f, result.dy, 0.0f });
            body = MoveBox(body, { 0.0f, result.dy });
        }

        actual_delta.y = result.dy;

        if (result.hit) {
            vel.linear.y = 0.0f;

            if (contact) {
                contact->hit_up = true;
            }
        }
    } else if (desired_delta.y < 0.0f) {
        // motor.min_ground_support
        auto result = ResolveDownMovement(body,
                                          desired_delta.y,
                                          tile_world,
                                          motor.min_ground_support);

        if (result.dy != 0.0f) {
            transform.Translate({ 0.0f, result.dy, 0.0f });
            body = MoveBox(body, { 0.0f, result.dy });
        }

        actual_delta.y = result.dy;

        if (result.hit) {
            vel.linear.y = 0.0f;

            if (contact) {
                contact->hit_down = true;
            }
        }
    }

    if (contact) {
        contact->actual_delta = actual_delta;
    }
}

}  // namespace cave
