#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/MovementComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"

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

struct CollisionPair {
    ecs::Entity a;
    ecs::Entity b;
    bool a_is_trigger;
    bool b_is_trigger;

    static CollisionPair make(ecs::Entity x,
                              ecs::Entity y,
                              bool x_is_trigger,
                              bool y_is_trigger) {
        return CollisionPair(x, y, x_is_trigger, y_is_trigger);
    }

private:
    CollisionPair(ecs::Entity x,
                  ecs::Entity y,
                  bool x_is_trigger,
                  bool y_is_trigger) {

        if (x > y) {
            std::swap(x, y);
            std::swap(x_is_trigger, y_is_trigger);
        }
        a = x;
        b = y;
        a_is_trigger = x_is_trigger;
        b_is_trigger = y_is_trigger;
    }
};

struct CollisionPairHash {
    std::size_t operator()(const CollisionPair& p) const noexcept {
        uint32_t a = p.a.id();
        uint32_t b = p.b.id();

        uint64_t packed = (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
        return std::hash<uint64_t>{}(packed);
    }
};

struct CollisionPairEqual {
    bool operator()(const CollisionPair& lhs, const CollisionPair& rhs) const noexcept {
        return lhs.a == rhs.a && lhs.b == rhs.b;
    }
};

class CollisionSystem {
    using TriggerCache = std::unordered_set<CollisionPair, CollisionPairHash, CollisionPairEqual>;

public:
    void runCollisionPair(SceneTickContext& ctx);

private:
    TriggerCache m_trigger_cache;
};

MotorSystem::MotorSystem()
    : m_debug_id(MakeDebugId(this))
    , m_collision(std::make_unique<CollisionSystem>()) {
}

MotorSystem::~MotorSystem() = default;

void MotorSystem::runTileWorldCollision(SceneTickContext& ctx) {
    const float dt = ctx.dt;
    Scene& scene = ctx.scene_ctx.scene;
    SceneQuery& query = ctx.scene_ctx.query;
    SceneRuntime& runtime = ctx.scene_ctx.runtime;

    const TileWorldSystem* tile_world = runtime.systems().get<TileWorldSystem>();
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

void CollisionSystem::runCollisionPair(SceneTickContext& ctx) {
    SceneQuery& query = ctx.scene_ctx.query;
    SceneRuntime& runtime = ctx.scene_ctx.runtime;
    NativeScriptSystem* script_system = runtime.systems().get<NativeScriptSystem>();
    if (!script_system) {
        return;
    }

    Scene& scene = ctx.scene_ctx.scene;
    std::vector<ColliderProxy> colliders;

    for (auto [ent, collider, transform] : scene.view<ColliderComponent, TransformComponent>()) {
        colliders.push_back({
            .entity = ent,
            .collider = &collider,
            .transform = &transform,
            .world_aabb = ComputeWorldAABB(transform, collider),
        });
    }

    TriggerCache cache;
    std::vector<CollisionPair> pairs;

    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            const ColliderProxy& a = colliders[i];
            const ColliderProxy& b = colliders[j];
            const bool a_is_trigger = a.collider->isTrigger();
            const bool b_is_trigger = b.collider->isTrigger();

            bool passes_filter =
                ((a.collider->mask() & b.collider->layer()) != 0) &&
                ((b.collider->mask() & a.collider->layer()) != 0) &&
                (a_is_trigger || b_is_trigger);

            if (!passes_filter) {
                continue;
            }

            if (!a.world_aabb.intersects(b.world_aabb)) {
                continue;
            }

            pairs.push_back(CollisionPair::make(
                a.entity,
                b.entity,
                a_is_trigger,
                b_is_trigger));
        }
    }

    auto resolve_script = [&query, &script_system](Entity e) -> NativeScript* {
        auto* comp = query.component<NativeScriptComponent>(e);
        return comp ? script_system->resolveScript(comp->handle) : nullptr;
    };

    auto fire_enter_or_stay = [&resolve_script, &ctx](bool is_trigger, Entity self, Entity other, bool was_overlapping) {
        if (!is_trigger) return;

        if (auto* instance = resolve_script(self)) {
            if (!was_overlapping) {
                instance->onBodyEntered(other);
            } else {
                instance->onBodyStay(other);
            }
        }
    };

    auto fire_exit = [&resolve_script, &ctx](bool is_trigger, Entity self, Entity other) {
        if (!is_trigger) return;
        if (auto* instance = resolve_script(self)) {
            instance->onBodyExited(other);
        }
    };

    for (const CollisionPair& pair : pairs) {
        cache.insert(pair);

        const bool in_last_frame = m_trigger_cache.find(pair) != m_trigger_cache.end();

        fire_enter_or_stay(pair.a_is_trigger, pair.a, pair.b, in_last_frame);
        fire_enter_or_stay(pair.b_is_trigger, pair.b, pair.a, in_last_frame);
    }

    for (const CollisionPair& pair : m_trigger_cache) {
        if (cache.find(pair) != cache.end()) {
            continue;
        }

        fire_exit(pair.a_is_trigger, pair.a, pair.b);
        fire_exit(pair.b_is_trigger, pair.b, pair.a);
    }

    m_trigger_cache = std::move(cache);
}

void MotorSystem::update(SceneTickContext& ctx) {
    runTileWorldCollision(ctx);
    m_collision->runCollisionPair(ctx);
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
