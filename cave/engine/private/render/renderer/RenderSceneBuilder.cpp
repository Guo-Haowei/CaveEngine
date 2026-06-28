#include "RenderSceneBuilder.h"

#include "RenderScene.h"

#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/SpriteRendererComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave::render {

using math::AABB;
using math::Mat4f;

void RenderSceneBuilder::BuildFull(const Scene& p_scene, RenderScene& p_out_scene) {
    p_out_scene.Clear();

    const size_t num_meshes = p_scene.count<MeshRendererComponent>();
    const size_t num_sprites = p_scene.count<SpriteRendererComponent>();
    const size_t num_tile_maps = 0;
    const size_t num_renderables = num_meshes + num_sprites + num_tile_maps;  // estimate
    p_out_scene.m_meshes.reserve(num_meshes);
    p_out_scene.m_sprites.reserve(num_sprites);
    p_out_scene.m_renderables.reserve(num_renderables);

    for (auto [e, renderer, trans] : p_scene.view<MeshRendererComponent, TransformComponent>()) {
        const MeshAsset* mesh_asset = renderer.GetMeshHandle().get();
        if (!mesh_asset) {
            continue;
        }

        PayloadRef ref{
            .kind = PayloadKind::Mesh,
            .index = static_cast<uint16_t>(p_out_scene.m_meshes.size()),
        };

        // MeshPayload
        p_out_scene.m_meshes.emplace_back(
            mesh_asset->gpuResource.get(),                      // mesh
            mesh_asset->localBound,                             // bound
            static_cast<uint32_t>(mesh_asset->indices.size()),  // index count
            renderer.GetSkeletonId()                            // skeleton
        );

        MeshPayload& payload = p_out_scene.m_meshes.back();
        const auto& materials = renderer.GetMaterialInstances();
        const size_t num_subset = mesh_asset->subsets.size();
        payload.subsets.resize(num_subset);
        payload.materials.reserve(num_subset);
        for (size_t idx = 0; idx < num_subset; ++idx) {
            payload.subsets[idx] = mesh_asset->subsets[idx];
            ecs::Entity material_id = idx < materials.size() ? materials[idx] : ecs::Entity::Null();
            payload.materials.emplace_back(material_id);
        }

        // Header
        RenderableFlags flags{};
        if (renderer.CastShadow()) flags |= RenderableFlags::CastShadow;
        if (renderer.Transparency()) flags |= RenderableFlags::Transparent;
        if (renderer.IsVisible()) flags |= RenderableFlags::Visible;
        if (payload.skeleton.IsValid()) flags |= RenderableFlags::Skinned;

        Mat4f world = trans.worldMatrix();
        AABB world_bound = payload.local_bound;
        world_bound.ApplyMatrix(world);
        p_out_scene.m_renderables.emplace_back(
            e,    // owner
            ref,  // payload
            flags,
            world,
            world_bound);
    }
}

void RenderSceneBuilder::OnEntityAdded(const Scene& p_scene, ecs::Entity p_ent, RenderScene& p_out_scene) {
    unused(p_scene);
    unused(p_ent);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnEntityRemoved(ecs::Entity p_ent, RenderScene& p_out_scene) {
    unused(p_ent);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnTransformChanged(ecs::Entity p_ent, RenderScene& p_out_scene) {
    unused(p_ent);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnMeshChanged(ecs::Entity p_ent, RenderScene& p_out_scene) {
    unused(p_ent);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnMaterialChanged(ecs::Entity p_ent, RenderScene& p_out_scene) {
    unused(p_ent);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnSkeletonChanged(ecs::Entity p_ent, RenderScene& p_out_scene) {
    unused(p_ent);
    unused(p_out_scene);
}

void RenderSceneBuilder::FlushPending(const Scene& p_scene, RenderScene& p_out_scene) {
    unused(p_scene);
    unused(p_out_scene);
}

}  // namespace cave::render
