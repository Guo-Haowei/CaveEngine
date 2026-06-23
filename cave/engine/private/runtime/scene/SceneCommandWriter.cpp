#include "cave/runtime/ecs/components/LightComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace cave::literals;
using ecs::Entity;

Entity SceneCommandWriter::CreateNameObject(std::string_view p_name) {
    Entity e = CreateEntity();
    AddComponent(e, NameComponent_Id);
    if (m_no_save) {
        AddComponent(e, NoSaveTag_Id);
    }
    SetProperty(e, NameComponent_Id, "name"_sid, FixedString<64>(p_name));
    return e;
}

Entity SceneCommandWriter::CreateRootObject(std::string_view p_name) {
    Entity e = CreateNameObject(p_name);
    AddComponent(e, TransformComponent_Id);
    return e;
}

Entity SceneCommandWriter::CreateTransformObject(std::string_view p_name) {
    Entity e = CreateNameObject(p_name);
    AddComponent(e, TransformComponent_Id);
    AddComponent(e, HierarchyComponent_Id);
    return e;
}

void SceneCommandWriter::AttachChild(ecs::Entity p_child, ecs::Entity p_parent) {
    DEV_ASSERT(p_child.IsValid() && p_parent.IsValid());
    SetProperty(p_child, HierarchyComponent_Id, "parent_id"_sid, p_parent);
}

Entity SceneCommandWriter::CreatePointLightObject(
    std::string_view p_name,
    const Vector3f& p_position,
    const Vector3f& p_color,
    float p_emissive) {
    SceneCommandBuffer cb;

    Entity e = CreateTransformObject(p_name);
    AddComponent(e, LightComponent_Id);
    AddComponent(e, MaterialComponent_Id);

    SetProperty(e, TransformComponent_Id, "translation"_sid, p_position);

    SetProperty(e, LightComponent_Id, "type"_sid, LightType::Point);
    SetProperty(e, LightComponent_Id, "atten_constant"_sid, 1.0f);
    SetProperty(e, LightComponent_Id, "atten_linear"_sid, 0.2f);
    SetProperty(e, LightComponent_Id, "atten_quadratic"_sid, 0.05f);

    SetProperty(e, MaterialComponent_Id, "base_color"_sid, Vector4f(p_color, 1.0f));
    SetProperty(e, MaterialComponent_Id, "emissive"_sid, p_emissive);

    return e;
}

Entity SceneCommandWriter::CreateInfiniteLightObject(std::string_view p_name,
                                                     const Vector3f& p_color,
                                                     float p_emissive) {
    Entity e = CreateTransformObject(p_name);
    AddComponent(e, LightComponent_Id);
    AddComponent(e, MaterialComponent_Id);

    SetProperty(e, LightComponent_Id, "type"_sid, LightType::Infinite);
    SetProperty(e, MaterialComponent_Id, "base_color"_sid, Vector4f(p_color, 1.0f));
    SetProperty(e, MaterialComponent_Id, "emissive"_sid, p_emissive);

    return e;
}

Entity SceneCommandWriter::CreateAreaLightObject(std::string_view p_name,
                                                 const Vector3f& p_color,
                                                 float p_emissive) {
    Entity e = CreateTransformObject(p_name);
    AddComponent(e, MeshRendererComponent_Id);
    AddComponent(e, LightComponent_Id);
    AddComponent(e, MaterialComponent_Id);

    SetProperty(e, LightComponent_Id, "type"_sid, LightType::Area);
    SetProperty(e, LightComponent_Id, "atten_constant"_sid, 1.0f);
    SetProperty(e, LightComponent_Id, "atten_linear"_sid, 0.09f);
    SetProperty(e, LightComponent_Id, "atten_quadratic"_sid, 0.032f);

    SetProperty(e, LightComponent_Id, "type"_sid, LightType::Infinite);
    SetProperty(e, MaterialComponent_Id, "base_color"_sid, Vector4f(p_color, 1.0f));
    SetProperty(e, MaterialComponent_Id, "emissive"_sid, p_emissive);

    auto handle = m_asset_reg.findByPath<MeshAsset>("@persist://meshes/plane").unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ e };
    SetProperty(e, MeshRendererComponent_Id, "mesh_id"_sid, handle.guid());
    SetProperty(e, MeshRendererComponent_Id, "materials"_sid, materials);

    return e;
}

Entity SceneCommandWriter::CreateMeshObject(const std::string& p_mesh_path,
                                            std::string_view p_name,
                                            const MaterialContext& p_mat_ctx) {
    Entity e = CreateTransformObject(p_name);

    AddComponent(e, MeshRendererComponent_Id);

    Entity mat = CreateNameObject(std::format("{}:mat", p_name));
    {
        AddComponent(mat, MaterialComponent_Id);
        if (p_mat_ctx.guid) {
            SetProperty(mat, MaterialComponent_Id, "material_id"_sid, *p_mat_ctx.guid);
        }
        if (p_mat_ctx.base_color != Vector4f::One) {
            SetProperty(mat, MaterialComponent_Id, "base_color"_sid, p_mat_ctx.base_color);
        }
    }

    auto handle = m_asset_reg.findByPath<MeshAsset>(p_mesh_path).unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ mat };
    SetProperty(e, MeshRendererComponent_Id, "mesh_id"_sid, handle.guid());
    SetProperty(e, MeshRendererComponent_Id, "materials"_sid, materials);

    return e;
}

Entity SceneCommandWriter::CreateMeshObject(const std::string& p_mesh_path,
                                            std::string_view p_name,
                                            const std::string& p_mat_path) {
    auto handle = m_asset_reg.findByPath<MaterialAsset>(p_mat_path);
    if (handle.is_some()) {
        const Guid guid = handle.unwrap_unchecked().guid();
        return CreateMeshObject(p_mesh_path, p_name, { &guid });
    }

    return CreateMeshObject(p_mesh_path, p_name, MaterialContext{ nullptr });
}

Entity SceneCommandWriter::CreatePlaneObject(std::string_view p_name, const MaterialContext& p_mat_ctx) {
    return CreateMeshObject("@persist://meshes/plane", p_name, p_mat_ctx);
}

Entity SceneCommandWriter::CreateCubeObject(std::string_view p_name, const MaterialContext& p_mat_ctx) {
    return CreateMeshObject("@persist://meshes/cube", p_name, p_mat_ctx);
}

Entity SceneCommandWriter::CreateSphereObject(std::string_view p_name, const MaterialContext& p_mat_ctx) {
    return CreateMeshObject("@persist://meshes/sphere", p_name, p_mat_ctx);
}

Entity SceneCommandWriter::CreateCylinderObject(std::string_view p_name, const MaterialContext& p_mat_ctx) {
    return CreateMeshObject("@persist://meshes/cylinder", p_name, p_mat_ctx);
}

Entity SceneCommandWriter::CreateConeObject(std::string_view p_name, const MaterialContext& p_mat_ctx) {
    return CreateMeshObject("@persist://meshes/cone", p_name, p_mat_ctx);
}

Entity SceneCommandWriter::CreateTorusObject(std::string_view p_name, const MaterialContext& p_mat_ctx) {
    return CreateMeshObject("@persist://meshes/torus", p_name, p_mat_ctx);
}

Entity SceneCommandWriter::CreateTileMapObject(std::string_view p_name) {
    Entity e = CreateTransformObject(p_name);
    AddComponent(e, TileMapInstanceComponent_Id);
    return e;
}

}  // namespace cave
