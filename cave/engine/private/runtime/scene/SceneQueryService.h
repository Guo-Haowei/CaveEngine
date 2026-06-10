#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

class SceneRegistry;

class SceneQueryService {
public:
    explicit SceneQueryService(SceneRegistry& scene_registry) noexcept
        : scene_registry_(scene_registry) {
    }

    RayHit raycast(SceneId scene_id,
                   math::Ray& ray,
                   const RaycastFilter& filter);

private:
    SceneRegistry& scene_registry_;
};

}  // namespace cave
