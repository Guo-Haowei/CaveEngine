#include "cave/runtime/ecs/components/LightComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneMutatorExt.h"

#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

using ecs::Entity;

Entity SceneExt::CreateNameObject(SceneCommandBuffer& p_cb, std::string_view p_name) {
    Entity e = p_cb.Create();
    p_cb.Add(e, NameComponent_Id);
    p_cb.SetProperty(e, NameComponent_Id, StringId("name"), FixedString<64>(p_name));
    return e;
}

Entity SceneExt::CreateRootObject(SceneCommandBuffer& p_cb, std::string_view p_name) {
    Entity e = CreateNameObject(p_cb, p_name);
    p_cb.Add(e, TransformComponent_Id);
    return e;
}

Entity SceneExt::CreateTransformObject(SceneCommandBuffer& p_cb, std::string_view p_name) {
    Entity e = CreateNameObject(p_cb, p_name);
    p_cb.Add(e, TransformComponent_Id);
    p_cb.Add(e, HierarchyComponent_Id);
    return e;
}

void SceneExt::AttachChild(SceneCommandBuffer& p_cb, ecs::Entity p_child, ecs::Entity p_parent) {
    DEV_ASSERT(p_child.IsValid() && p_parent.IsValid());
    p_cb.SetProperty(p_child, HierarchyComponent_Id, StringId("parent_id"), p_parent);
}

Entity SceneExt::CreatePointLightObject(SceneCommandBuffer& p_cb,
                                        std::string_view p_name,
                                        const Vector3f& p_position,
                                        const Vector3f& p_color,
                                        float p_emissive) {
    SceneCommandBuffer cb;

    Entity e = CreateTransformObject(p_cb, p_name);
    p_cb.Add(e, LightComponent_Id);
    p_cb.Add(e, MaterialComponent_Id);

    p_cb.SetProperty(e, TransformComponent_Id, StringId("translation"), p_position);

    p_cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Point);
    p_cb.SetProperty(e, LightComponent_Id, StringId("atten_constant"), 1.0f);
    p_cb.SetProperty(e, LightComponent_Id, StringId("atten_linear"), 0.2f);
    p_cb.SetProperty(e, LightComponent_Id, StringId("atten_quadratic"), 0.05f);

    p_cb.SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    p_cb.SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    return e;
}

Entity SceneExt::CreateInfiniteLightObject(SceneCommandBuffer& p_cb,
                                           std::string_view p_name,
                                           const Vector3f& p_color,
                                           float p_emissive) {
    Entity e = CreateTransformObject(p_cb, p_name);
    p_cb.Add(e, LightComponent_Id);
    p_cb.Add(e, MaterialComponent_Id);

    p_cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Infinite);
    p_cb.SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    p_cb.SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    return e;
}

Entity SceneExt::CreateAreaLightObject(SceneCommandBuffer& p_cb,
                                       std::string_view p_name,
                                       const Vector3f& p_color,
                                       float p_emissive) {
    Entity e = CreateTransformObject(p_cb, p_name);
    p_cb.Add(e, MeshRendererComponent_Id);
    p_cb.Add(e, LightComponent_Id);
    p_cb.Add(e, MaterialComponent_Id);

    p_cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Area);
    p_cb.SetProperty(e, LightComponent_Id, StringId("atten_constant"), 1.0f);
    p_cb.SetProperty(e, LightComponent_Id, StringId("atten_linear"), 0.09f);
    p_cb.SetProperty(e, LightComponent_Id, StringId("atten_quadratic"), 0.032f);

    p_cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Infinite);
    p_cb.SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    p_cb.SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    auto handle = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/plane").unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ e };
    p_cb.SetProperty(e, MeshRendererComponent_Id, StringId("mesh_id"), handle.GetGuid());
    p_cb.SetProperty(e, MeshRendererComponent_Id, StringId("materials"), materials);

    return e;
}

static Entity CreateMeshObject(const std::string& p_asset_path,
                               SceneCommandBuffer& p_cb,
                               std::string_view p_name,
                               const Guid* p_mat_guid) {

    Entity e = SceneExt::CreateTransformObject(p_cb, p_name);

    p_cb.Add(e, MeshRendererComponent_Id);

    Entity mat = SceneExt::CreateNameObject(p_cb, std::format("{}:mat", p_name));
    {
        p_cb.Add(mat, MaterialComponent_Id);
        if (p_mat_guid) {
            p_cb.SetProperty(mat, MaterialComponent_Id, StringId("material_id"), *p_mat_guid);
        }
    }

    auto handle = AssetRegistry::GetSingleton().FindByPath<MeshAsset>(p_asset_path).unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ mat };
    p_cb.SetProperty(e, MeshRendererComponent_Id, StringId("mesh_id"), handle.GetGuid());
    p_cb.SetProperty(e, MeshRendererComponent_Id, StringId("materials"), materials);

    return e;
}

Entity SceneExt::CreatePlaneObject(SceneCommandBuffer& p_cb,
                                   std::string_view p_name,
                                   const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/plane", p_cb, p_name, p_mat_guid);
}

Entity SceneExt::CreateCubeObject(SceneCommandBuffer& p_cb,
                                  std::string_view p_name,
                                  const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/cube", p_cb, p_name, p_mat_guid);
}

Entity SceneExt::CreateSphereObject(SceneCommandBuffer& p_cb,
                                    std::string_view p_name,
                                    const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/sphere", p_cb, p_name, p_mat_guid);
}

Entity SceneExt::CreateCylinderObject(SceneCommandBuffer& p_cb,
                                      std::string_view p_name,
                                      const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/cylinder", p_cb, p_name, p_mat_guid);
}

Entity SceneExt::CreateConeObject(SceneCommandBuffer& p_cb,
                                  std::string_view p_name,
                                  const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/cone", p_cb, p_name, p_mat_guid);
}

Entity SceneExt::CreateTorusObject(SceneCommandBuffer& p_cb,
                                   std::string_view p_name,
                                   const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/torus", p_cb, p_name, p_mat_guid);
}

Entity SceneExt::CreateTileMapObject(SceneCommandBuffer& p_cb,
                                     std::string_view p_name) {
    Entity e = CreateTransformObject(p_cb, p_name);
    p_cb.Add(e, TileMapRendererComponent_Id);
    return e;
}

}  // namespace cave
