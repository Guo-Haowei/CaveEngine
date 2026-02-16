#include "cave/runtime/ecs/components/LightComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;

Entity SceneCommandWriter::CreateNameObject(std::string_view p_name) {
    Entity e = Create();
    Add(e, NameComponent_Id);
    if (m_no_save) {
        Add(e, NoSaveTag_Id);
    }
    SetProperty(e, NameComponent_Id, StringId("name"), FixedString<64>(p_name));
    return e;
}

Entity SceneCommandWriter::CreateRootObject(std::string_view p_name) {
    Entity e = CreateNameObject(p_name);
    Add(e, TransformComponent_Id);
    return e;
}

Entity SceneCommandWriter::CreateTransformObject(std::string_view p_name) {
    Entity e = CreateNameObject(p_name);
    Add(e, TransformComponent_Id);
    Add(e, HierarchyComponent_Id);
    return e;
}

void SceneCommandWriter::AttachChild(ecs::Entity p_child, ecs::Entity p_parent) {
    DEV_ASSERT(p_child.IsValid() && p_parent.IsValid());
    SetProperty(p_child, HierarchyComponent_Id, StringId("parent_id"), p_parent);
}

Entity SceneCommandWriter::CreatePointLightObject(
    std::string_view p_name,
    const Vector3f& p_position,
    const Vector3f& p_color,
    float p_emissive) {
    SceneCommandBuffer cb;

    Entity e = CreateTransformObject(p_name);
    Add(e, LightComponent_Id);
    Add(e, MaterialComponent_Id);

    SetProperty(e, TransformComponent_Id, StringId("translation"), p_position);

    SetProperty(e, LightComponent_Id, StringId("type"), LightType::Point);
    SetProperty(e, LightComponent_Id, StringId("atten_constant"), 1.0f);
    SetProperty(e, LightComponent_Id, StringId("atten_linear"), 0.2f);
    SetProperty(e, LightComponent_Id, StringId("atten_quadratic"), 0.05f);

    SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    return e;
}

Entity SceneCommandWriter::CreateInfiniteLightObject(std::string_view p_name,
                                                     const Vector3f& p_color,
                                                     float p_emissive) {
    Entity e = CreateTransformObject(p_name);
    Add(e, LightComponent_Id);
    Add(e, MaterialComponent_Id);

    SetProperty(e, LightComponent_Id, StringId("type"), LightType::Infinite);
    SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    return e;
}

Entity SceneCommandWriter::CreateAreaLightObject(std::string_view p_name,
                                                 const Vector3f& p_color,
                                                 float p_emissive) {
    Entity e = CreateTransformObject(p_name);
    Add(e, MeshRendererComponent_Id);
    Add(e, LightComponent_Id);
    Add(e, MaterialComponent_Id);

    SetProperty(e, LightComponent_Id, StringId("type"), LightType::Area);
    SetProperty(e, LightComponent_Id, StringId("atten_constant"), 1.0f);
    SetProperty(e, LightComponent_Id, StringId("atten_linear"), 0.09f);
    SetProperty(e, LightComponent_Id, StringId("atten_quadratic"), 0.032f);

    SetProperty(e, LightComponent_Id, StringId("type"), LightType::Infinite);
    SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    auto handle = m_asset_reg.FindByPath<MeshAsset>("@persist://meshes/plane").unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ e };
    SetProperty(e, MeshRendererComponent_Id, StringId("mesh_id"), handle.GetGuid());
    SetProperty(e, MeshRendererComponent_Id, StringId("materials"), materials);

    return e;
}

Entity SceneCommandWriter::CreateMeshObject(const std::string& p_mesh_path,
                                            std::string_view p_name,
                                            const Guid* p_mat_guid) {
    Entity e = CreateTransformObject(p_name);

    Add(e, MeshRendererComponent_Id);

    Entity mat = CreateNameObject(std::format("{}:mat", p_name));
    {
        Add(mat, MaterialComponent_Id);
        if (p_mat_guid) {
            SetProperty(mat, MaterialComponent_Id, StringId("material_id"), *p_mat_guid);
        }
    }

    auto handle = m_asset_reg.FindByPath<MeshAsset>(p_mesh_path).unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ mat };
    SetProperty(e, MeshRendererComponent_Id, StringId("mesh_id"), handle.GetGuid());
    SetProperty(e, MeshRendererComponent_Id, StringId("materials"), materials);

    return e;
}

Entity SceneCommandWriter::CreateMeshObject(const std::string& p_mesh_path,
                                            std::string_view p_name,
                                            const std::string& p_mat_path) {
    auto handle = m_asset_reg.FindByPath<MaterialAsset>(p_mat_path);
    if (handle.is_some()) {
        const Guid guid = handle.unwrap_unchecked().GetGuid();
        return CreateMeshObject(p_mesh_path, p_name, &guid);
    }

    return CreateMeshObject(p_mesh_path, p_name, nullptr);
}

Entity SceneCommandWriter::CreatePlaneObject(std::string_view p_name, const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/plane", p_name, p_mat_guid);
}

Entity SceneCommandWriter::CreateCubeObject(std::string_view p_name, const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/cube", p_name, p_mat_guid);
}

Entity SceneCommandWriter::CreateSphereObject(std::string_view p_name, const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/sphere", p_name, p_mat_guid);
}

Entity SceneCommandWriter::CreateCylinderObject(std::string_view p_name, const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/cylinder", p_name, p_mat_guid);
}

Entity SceneCommandWriter::CreateConeObject(std::string_view p_name, const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/cone", p_name, p_mat_guid);
}

Entity SceneCommandWriter::CreateTorusObject(std::string_view p_name, const Guid* p_mat_guid) {
    return CreateMeshObject("@persist://meshes/torus", p_name, p_mat_guid);
}

Entity SceneCommandWriter::CreateTileMapObject(std::string_view p_name) {
    Entity e = CreateTransformObject(p_name);
    Add(e, TileMapRendererComponent_Id);
    return e;
}

}  // namespace cave
