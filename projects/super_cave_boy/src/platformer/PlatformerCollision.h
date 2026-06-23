#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace super_cave_boy {

struct VerticalMoveResult {
    bool hit = false;
    float dy = 0.0f;
};

inline bool Overlap1DStrict(float a_min, float a_max, float b_min, float b_max) {
    return a_max > b_min && a_min < b_max;
}

inline float OverlapAmount1D(float a_min, float a_max, float b_min, float b_max) {
    return std::min(a_max, b_max) - std::max(a_min, b_min);
}

inline cave::math::Box2 MoveBox(cave::math::Box2 box, cave::math::Vec2f delta) {
    box.SetMinMax(box.Min() + delta, box.Max() + delta);
    return box;
}

cave::math::Box2 ComputeWorldAABB(const cave::TransformComponent& transform,
                                  const cave::ColliderComponent& collider);

float ResolveHorizontalMovement(const cave::math::Box2& body,
                                float dx,
                                const cave::TileWorldSystem& world);

VerticalMoveResult ResolveUpMovement(const cave::math::Box2& body,
                                     float dy,
                                     const cave::TileWorldSystem& world);

}  // namespace super_cave_boy
