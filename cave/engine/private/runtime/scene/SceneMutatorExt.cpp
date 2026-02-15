#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneMutatorExt.h"

namespace cave {

using ecs::Entity;

Entity CreateNameObject(SceneCommandBuffer& p_cb, std::string_view p_name) {
    Entity e = p_cb.Create();
    p_cb.Add(e, NameComponent_Id);
    p_cb.SetProperty(e, NameComponent_Id, StringId("name"), FixedString<64>(p_name));
    return e;
}

Entity CreateTransformObject(SceneCommandBuffer& p_cb, std::string_view p_name) {
    Entity e = CreateNameObject(p_cb, p_name);
    p_cb.Add(e, TransformComponent_Id);
    return e;
}

Entity CreateMeshObject(SceneCommandBuffer& p_cb, std::string_view p_name) {
    Entity e = CreateNameObject(p_cb, p_name);
    p_cb.Add(e, TransformComponent_Id);
    p_cb.Add(e, MeshRendererComponent_Id);
    return e;
}

}  // namespace cave
