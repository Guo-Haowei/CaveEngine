#include "PlatformerCollision.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

Box2 ComputeWorldAABB(const TransformComponent& transform,
                      const ColliderComponent& collider) {
    const Shape& shape = collider.shape();
    Vec2f translation = transform.GetTranslation().xy;

    return {
        translation - Vec2f(shape.data.half.xy),
        translation + Vec2f(shape.data.half.xy),
    };
}

}  // namespace super_cave_boy
