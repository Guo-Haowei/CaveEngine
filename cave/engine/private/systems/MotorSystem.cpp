#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/script/native/NativeScriptComponent.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;
using ::cave::ecs::Entity;

Box2 ComputeWorldAABB(const TransformComponent& transform,
                      const ColliderComponent& collider) {
    const Shape& shape = collider.shape();
    Vec2f translation = transform.translation().xy;

    return {
        translation - Vec2f(shape.data.half.xy),
        translation + Vec2f(shape.data.half.xy),
    };
}

namespace {

cave::math::Box2 MoveBox(cave::math::Box2 box, cave::math::Vec2f delta) {
    box.setMinMax(box.min() + delta, box.max() + delta);
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
    query.expandToInclude(body);

    float resolved_dx = dx;

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        if (!Overlap1DStrict(body.min().y, body.max().y,
                             solid.min().y, solid.max().y)) {
            continue;
        }

        if (dx > 0.0f) {
            if (body.max().x <= solid.min().x) {
                const float candidate_dx = solid.min().x - body.max().x;
                resolved_dx = std::min(resolved_dx, candidate_dx);
            }
        } else {
            if (body.min().x >= solid.max().x) {
                const float candidate_dx = solid.max().x - body.min().x;
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
    query.expandToInclude(body);

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        const bool x_overlap =
            body.max().x - step_offset >= solid.min().x &&
            body.min().x + step_offset <= solid.max().x;

        if (!x_overlap) {
            continue;
        }

        if (body.max().y <= solid.min().y && body.max().y + dy >= solid.min().y) {
            result.hit = true;
            result.dy = std::min(result.dy, solid.min().y - body.max().y);
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
    query.expandToInclude(body);

    for (const TileHit& hit_tile : world.querySolidTiles(query)) {
        const Box2& solid = hit_tile.aabb;

        const float overlap_x = OverlapAmount1D(
            body.min().x,
            body.max().x,
            solid.min().x,
            solid.max().x);

        if (overlap_x <= min_ground_support) {
            continue;
        }

        constexpr float kSkin = 0.001f;
        if (body.min().y >= solid.max().y - kSkin &&
            body.min().y + dy <= solid.max().y + kSkin) {
            result.hit = true;
            result.dy = std::max(result.dy, solid.max().y - body.min().y);
        }
    }

    return result;
}

}  // namespace

MotorSystem::MotorSystem()
    : debug_id_(MakeDebugId(this)) {
}

void MotorSystem::runTileWorldCollision(SceneQuery& query, float dt) {
    Scene& scene = context().scene;
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

struct ColliderProxy {
    ecs::Entity entity;
    const ColliderComponent* collider = nullptr;
    const TransformComponent* transform = nullptr;
    Box2 world_aabb;
    bool is_trigger = false;
};

struct CollisionPair {
    ecs::Entity a;
    ecs::Entity b;
};

void MotorSystem::runCollisionPair(SceneQuery& query, float) {
    Scene& scene = context().scene;

    std::vector<ColliderProxy> colliders;

    for (auto [ent, collider, transform] : scene.view<ColliderComponent, TransformComponent>()) {
        colliders.push_back({
            .entity = ent,
            .collider = &collider,
            .transform = &transform,
            .world_aabb = ComputeWorldAABB(transform, collider),
        });
    }

    std::vector<CollisionPair> pairs;

    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            const ColliderProxy& a = colliders[i];
            const ColliderProxy& b = colliders[j];

            bool passes_filter =
                ((a.collider->mask() & b.collider->layer()) != 0) &&
                ((b.collider->mask() & a.collider->layer()) != 0);

            if (!passes_filter) {
                continue;
            }

            if (!a.world_aabb.intersects(b.world_aabb)) {
                continue;
            }

            pairs.push_back({
                .a = a.entity,
                .b = b.entity,
            });
        }
    }

    for (const auto& pair : pairs) {
        auto a = query.component<NativeScriptComponent>(pair.a);
        if (a && a->instance) {
            a->instance->onCollision(pair.b);
        }
        auto b = query.component<NativeScriptComponent>(pair.b);
        if (b && b->instance) {
            b->instance->onCollision(pair.a);
        }
    }
}

void MotorSystem::update(float dt) {
    Scene& scene = context().scene;
    SceneQuery query(scene);

    runTileWorldCollision(query, dt);
    runCollisionPair(query, dt);
}

void MotorSystem::moveKinematic2D(const TileWorldSystem& tile_world,
                                  TransformComponent& transform,
                                  VelocityComponent& vel,
                                  const ColliderComponent& collider,
                                  const MotorComponent& motor,
                                  ContactComponent* out_contact,
                                  Vec2f desired_delta) {
    Box2 body = ComputeWorldAABB(transform, collider);

    Vec2f actual_delta{ 0.0f, 0.0f };

    ContactComponent contact{};

    if (desired_delta.x != 0.0f) {
        float dx = ResolveHorizontalMovement(body, desired_delta.x, tile_world);

        if (dx != 0.0f) {
            transform.translate({ dx, 0.0f, 0.0f });
            body = MoveBox(body, { dx, 0.0f });
        }

        actual_delta.x = dx;

        if (dx != desired_delta.x) {
            vel.linear.x = 0.0f;

            if (desired_delta.x < 0.0f) {
                contact.hit_left = true;
            } else {
                contact.hit_right = true;
            }
        }
    }

    if (desired_delta.y > 0.0f) {
        auto result = ResolveUpMovement(body,
                                        desired_delta.y,
                                        tile_world,
                                        motor.step_offset);

        if (result.dy != 0.0f) {
            transform.translate({ 0.0f, result.dy, 0.0f });
            body = MoveBox(body, { 0.0f, result.dy });
        }

        actual_delta.y = result.dy;

        if (result.hit) {
            vel.linear.y = 0.0f;
            contact.hit_up = true;
        }
    } else if (desired_delta.y < 0.0f) {
        auto result = ResolveDownMovement(body,
                                          desired_delta.y,
                                          tile_world,
                                          motor.min_ground_support);

        if (result.dy != 0.0f) {
            transform.translate({ 0.0f, result.dy, 0.0f });
            body = MoveBox(body, { 0.0f, result.dy });
        }

        actual_delta.y = result.dy;

        if (result.hit) {
            vel.linear.y = 0.0f;

            contact.hit_down = true;
        }
    }

    contact.actual_delta = actual_delta;
    if (out_contact) {
        *out_contact = contact;
    }
}

}  // namespace cave
