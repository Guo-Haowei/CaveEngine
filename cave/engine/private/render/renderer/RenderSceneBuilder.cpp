#include "RenderSceneBuilder.h"

namespace cave::render {

void RenderSceneBuilder::BuildFull(const Scene& p_scene, RenderScene& p_out_scene) {
    unused(p_scene);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnEntityAdded(const Scene& p_scene, ecs::Entity p_entity, RenderScene& p_out_scene) {
    unused(p_scene);
    unused(p_entity);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnEntityRemoved(ecs::Entity p_entity, RenderScene& p_out_scene) {
    unused(p_entity);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnTransformChanged(ecs::Entity p_entity, RenderScene& p_out_scene) {
    unused(p_entity);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnMeshChanged(ecs::Entity p_entity, RenderScene& p_out_scene) {
    unused(p_entity);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnMaterialChanged(ecs::Entity p_entity, RenderScene& p_out_scene) {
    unused(p_entity);
    unused(p_out_scene);
}

void RenderSceneBuilder::OnSkeletonChanged(ecs::Entity p_entity, RenderScene& p_out_scene) {
    unused(p_entity);
    unused(p_out_scene);
}

void RenderSceneBuilder::FlushPending(const Scene& p_scene, RenderScene& p_out_scene) {
    unused(p_scene);
    unused(p_out_scene);
}

}  // namespace cave::render
