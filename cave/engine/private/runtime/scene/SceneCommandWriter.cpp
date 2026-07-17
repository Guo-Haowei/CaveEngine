#include "cave/runtime/ecs/components/LightComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;
using ecs::Entity;

Entity SceneCommandWriter::nameObject(std::string_view name) {
    Entity e = createEntity();
    addComponent(e, NameComponent_Id);
    if (m_no_save) {
        addComponent(e, PrefabChildComponent_Id);
    }
    setProperty(e, NameComponent_Id, CAVE_SID("name"), FixedString<64>(name));
    return e;
}

Entity SceneCommandWriter::rootObject(std::string_view name) {
    Entity e = nameObject(name);
    addComponent(e, TransformComponent_Id);
    addComponent(e, HierarchyComponent_Id);
    return e;
}

Entity SceneCommandWriter::prefabObject(std::string_view name, const Guid& guid) {
    Entity e = transformObject(name);
    addComponent(e, PrefabInstanceComponent_Id);
    if (!guid.isNull()) {
        setProperty(e, PrefabInstanceComponent_Id, CAVE_SID("prefab_id"), guid);
    }
    return e;
}

Entity SceneCommandWriter::transformObject(std::string_view name) {
    Entity e = nameObject(name);
    addComponent(e, TransformComponent_Id);
    addComponent(e, HierarchyComponent_Id);
    return e;
}

void SceneCommandWriter::attachChild(ecs::Entity child, ecs::Entity parent) {
    DEV_ASSERT(child.valid() && parent.valid());
    setProperty(child, HierarchyComponent_Id, CAVE_SID("parent_id"), parent);
}

Entity SceneCommandWriter::pointLightObject(
    std::string_view name,
    const Vec3f& position,
    const Vec3f& color,
    float emissive) {
    SceneCommandBuffer cb;

    Entity e = transformObject(name);
    addComponent(e, LightComponent_Id);
    addComponent(e, MaterialComponent_Id);

    setProperty(e, TransformComponent_Id, CAVE_SID("translation"), position);

    setProperty(e, LightComponent_Id, CAVE_SID("type"), LightType::Point);
    setProperty(e, LightComponent_Id, CAVE_SID("atten_constant"), 1.0f);
    setProperty(e, LightComponent_Id, CAVE_SID("atten_linear"), 0.2f);
    setProperty(e, LightComponent_Id, CAVE_SID("atten_quadratic"), 0.05f);

    setProperty(e, MaterialComponent_Id, CAVE_SID("base_color"), Vec4f(color, 1.0f));
    setProperty(e, MaterialComponent_Id, CAVE_SID("emissive"), emissive);

    return e;
}

Entity SceneCommandWriter::infiniteLightObject(std::string_view name,
                                               const Vec3f& color,
                                               float emissive) {
    Entity e = transformObject(name);
    addComponent(e, LightComponent_Id);
    addComponent(e, MaterialComponent_Id);

    setProperty(e, LightComponent_Id, CAVE_SID("type"), LightType::Infinite);
    setProperty(e, MaterialComponent_Id, CAVE_SID("base_color"), Vec4f(color, 1.0f));
    setProperty(e, MaterialComponent_Id, CAVE_SID("emissive"), emissive);

    return e;
}

Entity SceneCommandWriter::areaLightObject(std::string_view name,
                                           const Vec3f& color,
                                           float emissive) {
    Entity e = transformObject(name);
    addComponent(e, MeshRendererComponent_Id);
    addComponent(e, LightComponent_Id);
    addComponent(e, MaterialComponent_Id);

    setProperty(e, LightComponent_Id, CAVE_SID("type"), LightType::Area);
    setProperty(e, LightComponent_Id, CAVE_SID("atten_constant"), 1.0f);
    setProperty(e, LightComponent_Id, CAVE_SID("atten_linear"), 0.09f);
    setProperty(e, LightComponent_Id, CAVE_SID("atten_quadratic"), 0.032f);

    setProperty(e, LightComponent_Id, CAVE_SID("type"), LightType::Infinite);
    setProperty(e, MaterialComponent_Id, CAVE_SID("base_color"), Vec4f(color, 1.0f));
    setProperty(e, MaterialComponent_Id, CAVE_SID("emissive"), emissive);

    auto handle = m_asset_reg.findByPath<MeshAsset>("@persist://meshes/plane").unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ e };
    setProperty(e, MeshRendererComponent_Id, CAVE_SID("mesh_guid"), handle.guid());
    setProperty(e, MeshRendererComponent_Id, CAVE_SID("materials"), materials);

    return e;
}

Entity SceneCommandWriter::meshObject(const std::string& mesh_path,
                                      std::string_view name,
                                      const MaterialContext& mat_ctx) {
    Entity e = transformObject(name);

    addComponent(e, MeshRendererComponent_Id);

    Entity mat = nameObject(std::format("{}:mat", name));
    {
        addComponent(mat, MaterialComponent_Id);
        if (mat_ctx.guid) {
            setProperty(mat, MaterialComponent_Id, CAVE_SID("material_id"), *mat_ctx.guid);
        }
        if (mat_ctx.base_color != Vec4f::One) {
            setProperty(mat, MaterialComponent_Id, CAVE_SID("base_color"), mat_ctx.base_color);
        }
    }

    auto handle = m_asset_reg.findByPath<MeshAsset>(mesh_path).unwrap();

    FixedStack<ecs::Entity, MeshRendererComponent::kMaxMaterial> materials{ mat };
    setProperty(e, MeshRendererComponent_Id, CAVE_SID("mesh_guid"), handle.guid());
    setProperty(e, MeshRendererComponent_Id, CAVE_SID("materials"), materials);

    return e;
}

Entity SceneCommandWriter::meshObject(const std::string& mesh_path,
                                      std::string_view name,
                                      const std::string& mat_path) {
    auto handle = m_asset_reg.findByPath<MaterialAsset>(mat_path);
    if (handle.is_some()) {
        const Guid guid = handle.unwrap_unchecked().guid();
        return meshObject(mesh_path, name, { &guid });
    }

    return meshObject(mesh_path, name, MaterialContext{ nullptr });
}

Entity SceneCommandWriter::tileMapObject(std::string_view name) {
    Entity e = transformObject(name);
    addComponent(e, TileMapInstanceComponent_Id);
    return e;
}

Entity SceneCommandWriter::canvas(std::string_view name) {
    Entity e = nameObject(name);
    addComponent(e, HierarchyComponent_Id);
    addComponent(e, UICanvasComponent_Id);
    return e;
}

Entity SceneCommandWriter::rect(std::string_view name) {
    Entity e = nameObject(name);
    addComponent(e, HierarchyComponent_Id);
    addComponent(e, UIRectTransformComponent_Id);
    return e;
}

Entity SceneCommandWriter::button(std::string_view name) {
    Entity e = nameObject(name);
    addComponent(e, HierarchyComponent_Id);
    addComponent(e, UIRectTransformComponent_Id);
    addComponent(e, UIButtonComponent_Id);
    return e;
}

Entity SceneCommandWriter::image(std::string_view name) {
    Entity e = nameObject(name);
    addComponent(e, HierarchyComponent_Id);
    addComponent(e, UIRectTransformComponent_Id);
    return e;
}

}  // namespace cave
