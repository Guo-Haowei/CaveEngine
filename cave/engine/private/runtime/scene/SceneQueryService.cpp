#pragma once
#include "SceneQueryService.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using namespace math;

RayHit SceneQueryService::raycast(SceneId scene_id,
                                  math::Ray& ray,
                                  const RaycastFilter& filter) {
    const Scene* scene = scene_registry_.Resolve(scene_id);
    DEV_ASSERT(scene);

    SceneQuery query(*scene);
    return query.raycast(ray, filter);
}

}  // namespace cave
