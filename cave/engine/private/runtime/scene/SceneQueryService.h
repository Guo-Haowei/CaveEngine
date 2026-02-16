#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace cave {

class SceneRegistry;

class SceneQueryService {
public:
    explicit SceneQueryService(SceneRegistry& p_scene_reg) noexcept
        : m_scene_reg(p_scene_reg) {
    }

    RayHit Raycast(SceneId p_scene_id,
                   math::Ray& p_ray,
                   const RaycastFilter& p_filter);

private:
    SceneRegistry& m_scene_reg;
};

}  // namespace cave
