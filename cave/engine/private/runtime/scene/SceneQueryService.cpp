#pragma once
#include "SceneQueryService.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using namespace math;

RayHit SceneQueryService::Raycast(SceneId p_scene_id,
                                  math::Ray& p_ray,
                                  const RaycastFilter& p_filter) {
    unused(p_filter);

    const Scene* scene = m_scene_reg.Resolve(p_scene_id);
    DEV_ASSERT(scene);

    SceneQuery query(*scene);
    return query.raycast(p_ray, p_filter);
}

}  // namespace cave
