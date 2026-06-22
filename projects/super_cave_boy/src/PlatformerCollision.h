#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

namespace super_cave_boy {

cave::math::Box2 ComputeWorldAABB(const cave::TransformComponent& transform,
                                  const cave::ColliderComponent& collider);

}  // namespace super_cave_boy
