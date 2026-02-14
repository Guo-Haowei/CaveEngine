#pragma once
#include "SceneQueryService.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"

#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace math;

static bool RaycastHelper(Ray& p_ray,
                          const MeshRendererComponent& p_mesh,
                          const TransformComponent& p_trans) {
    MeshAsset* mesh = p_mesh.GetMeshHandle().Get();
    if (!mesh) return false;

    Matrix4x4f model_inv = glm::inverse(p_trans.GetWorldMatrix());
    Ray ray_inv = p_ray.Inverse(model_inv);
    // make a copy, so aabb test doesn't change t
    if (!Ray(ray_inv).Intersects(mesh->localBound)) {
        return false;
    }

    // Test every single triange
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        const Vector3f& A = mesh->positions[mesh->indices[i]];
        const Vector3f& B = mesh->positions[mesh->indices[i + 1]];
        const Vector3f& C = mesh->positions[mesh->indices[i + 2]];
        if (ray_inv.Intersects(A, B, C)) {
            p_ray.SetDist(ray_inv.GetDist());
            return true;
        }
    }
    return false;
}

RayHit SceneQueryService::Raycast(SceneId p_scene_id,
                                  math::Ray& p_ray,
                                  const RaycastFilter& p_filter) {
    unused(p_filter);

    const Scene* scene = m_scene_reg.Resolve(p_scene_id);
    DEV_ASSERT(scene);

    RayHit res{};
    for (auto [entity, mesh, transform] : scene->View<MeshRendererComponent, TransformComponent>()) {
        if (RaycastHelper(p_ray, mesh, transform)) {
            res.hit = true;
            res.entity = entity;
            res.t = p_ray.GetDist();
        }
    }

    return res;
}

}  // namespace cave
