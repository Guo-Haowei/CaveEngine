#pragma once
#include "cave/runtime/ecs/Entity.h"

// clang-format off
namespace cave { class Scene; }
// clang-format on

namespace cave::render {

class RenderScene;

class RenderSceneBuilder {
public:
    // @TODO: start with force syncing every frame
    void BuildFull(const Scene& p_scene, RenderScene& p_out_scene);

    void OnEntityAdded(const Scene& p_scene, ecs::Entity p_ent, RenderScene& p_out_scene);
    void OnEntityRemoved(ecs::Entity p_ent, RenderScene& p_out_scene);

    void OnTransformChanged(ecs::Entity p_ent, RenderScene& p_out_scene);
    void OnMeshChanged(ecs::Entity p_ent, RenderScene& p_out_scene);
    void OnMaterialChanged(ecs::Entity p_ent, RenderScene& p_out_scene);
    void OnSkeletonChanged(ecs::Entity p_ent, RenderScene& p_out_scene);

    void FlushPending(const Scene& p_scene, RenderScene& p_out_scene);
};

}  // namespace cave::render
