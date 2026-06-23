#include "PlatformerCollision.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

namespace {
constexpr float kStepOffset = 0.05f;
}  // namespace

Box2 ComputeWorldAABB(const TransformComponent& transform,
                      const ColliderComponent& collider) {
    const Shape& shape = collider.shape();
    Vec2f translation = transform.GetTranslation().xy;

    return {
        translation - Vec2f(shape.data.half.xy),
        translation + Vec2f(shape.data.half.xy),
    };
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

VerticalMoveResult ResolveUpMovement(const Box2& body,
                                     float dy,
                                     const TileWorldSystem& world) {
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
            body.Max().x - kStepOffset >= solid.Min().x &&
            body.Min().x + kStepOffset <= solid.Max().x;

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


}  // namespace super_cave_boy
