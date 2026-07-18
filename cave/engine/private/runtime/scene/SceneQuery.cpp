#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/render/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/MiscComponents.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneSerializer.h"

namespace cave {

using namespace math;
using ecs::Entity;

Entity SceneQuery::findFirstByName(std::string_view name) const {
    return m_scene.findFirstByName(name);
}

Entity SceneQuery::findChildByName(std::string_view name, Entity ent) const {
    return m_scene.findChildByName(name, ent);
}

void SceneQuery::queueDestroy(ecs::Entity ent) {
    m_scene.create<PendingDestroyComponent>(ent);
}

void* SceneQuery::addComponent(ComponentId cid, ecs::Entity ent) {
    return m_scene.storage().createRaw(cid, ent);
}

void* SceneQuery::component(ComponentId cid, Entity ent) {
    return m_scene.storage().getRaw(cid, ent);
}

const void* SceneQuery::component(ComponentId cid, Entity ent) const {
    return m_scene.storage().getRaw(cid, ent);
}

size_t SceneQuery::componentCount(ComponentId cid) const {
    return m_scene.count(cid);
}

static bool RaycastHelper(Ray& ray,
                          const MeshAsset& mesh,
                          const TransformComponent& transform) {

    Mat4f model_inv = glm::inverse(transform.worldMatrix());
    Ray ray_inv = ray.inverse(model_inv);
    // make a copy, so aabb test doesn't change t
    if (!Ray(ray_inv).intersects(mesh.localBound)) {
        return false;
    }

    // Test every single triange
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        const Vec3f& A = mesh.positions[mesh.indices[i]];
        const Vec3f& B = mesh.positions[mesh.indices[i + 1]];
        const Vec3f& C = mesh.positions[mesh.indices[i + 2]];
        if (ray_inv.intersects(A, B, C)) {
            ray.distance(ray_inv.distance());
            return true;
        }
    }
    return false;
}

RayHit SceneQuery::raycast(math::Ray& ray, const RaycastFilter&) const {
    RayHit res{};
    for (auto [entity, mesh, transform] : m_scene.view<MeshRendererComponent, TransformComponent>()) {
        MeshAsset* mesh_asset = mesh.meshHandle().get();
        if (!mesh_asset) continue;
        if (!RaycastHelper(ray, *mesh_asset, transform)) continue;
        res.hit = true;
        res.entity = entity;
        res.t = ray.distance();
    }

    return res;
}

}  // namespace cave
