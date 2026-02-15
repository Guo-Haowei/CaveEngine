#include "EntityFactory.h"

#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneMutator.h"
#include "cave/runtime/scene/SceneMutatorExt.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

using ecs::Entity;
using namespace ::cave::math;

Entity EntityFactory::CreateNameEntity(Scene& p_scene, std::string_view p_name) {
    SceneCommandBuffer cb;
    Entity e = CreateNameObject(cb, p_name);

    SceneMutator mut(p_scene);
    cb.Playback(mut);
    return cb.Resolve(e);
}

Entity EntityFactory::CreateTransformEntity(Scene& p_scene, std::string_view p_name) {
    SceneCommandBuffer cb;
    Entity e = CreateTransformObject(cb, p_name);

    SceneMutator mut(p_scene);
    cb.Playback(mut);
    return cb.Resolve(e);
}

Entity EntityFactory::CreatePointLightEntity(Scene& p_scene,
                                             const std::string& p_name,
                                             const Vector3f& p_position,
                                             const Vector3f& p_color,
                                             float p_emissive) {
    SceneCommandBuffer cb;

    Entity e = CreateNameObject(cb, p_name);
    cb.Add(e, TransformComponent_Id);
    cb.Add(e, MeshRendererComponent_Id);
    cb.Add(e, LightComponent_Id);
    cb.Add(e, MaterialComponent_Id);

    cb.SetProperty(e, TransformComponent_Id, StringId("translation"), p_position);

    cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Point);
    cb.SetProperty(e, LightComponent_Id, StringId("atten_constant"), 1.0f);
    cb.SetProperty(e, LightComponent_Id, StringId("atten_linear"), 0.2f);
    cb.SetProperty(e, LightComponent_Id, StringId("atten_quadratic"), 0.05f);

    cb.SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    cb.SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    SceneMutator mut(p_scene);
    cb.Playback(mut);
    return cb.Resolve(e);
}

Entity EntityFactory::CreateInfiniteLightEntity(Scene& p_scene,
                                                const std::string& p_name,
                                                const Vector3f& p_color,
                                                float p_emissive) {
    SceneCommandBuffer cb;
    Entity e = CreateNameObject(cb, p_name);
    cb.Add(e, TransformComponent_Id);
    cb.Add(e, LightComponent_Id);
    cb.Add(e, MaterialComponent_Id);

    cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Infinite);
    cb.SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    cb.SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    SceneMutator mut(p_scene);
    cb.Playback(mut);
    return cb.Resolve(e);
}

Entity EntityFactory::CreateAreaLightEntity(Scene& p_scene,
                                            const std::string& p_name,
                                            const Vector3f& p_color,
                                            float p_emissive) {
    SceneCommandBuffer cb;
    Entity e = CreateNameObject(cb, p_name);
    cb.Add(e, TransformComponent_Id);
    cb.Add(e, MeshRendererComponent_Id);
    cb.Add(e, LightComponent_Id);
    cb.Add(e, MaterialComponent_Id);

    cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Area);
    cb.SetProperty(e, LightComponent_Id, StringId("atten_constant"), 1.0f);
    cb.SetProperty(e, LightComponent_Id, StringId("atten_linear"), 0.09f);
    cb.SetProperty(e, LightComponent_Id, StringId("atten_quadratic"), 0.032f);

    cb.SetProperty(e, LightComponent_Id, StringId("type"), LightType::Infinite);
    cb.SetProperty(e, MaterialComponent_Id, StringId("base_color"), Vector4f(p_color, 1.0f));
    cb.SetProperty(e, MaterialComponent_Id, StringId("emissive"), p_emissive);

    auto handle = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/plane").unwrap();
    cb.SetProperty(e, MeshRendererComponent_Id, StringId("mesh_id"), handle.GetGuid());

#if 0
    // mesh
    MeshRendererComponent& renderer = *p_scene.GetComponent<MeshRendererComponent>(id);
    renderer.AddMaterial(id);

    renderer.SetResourceGuid(handle.GetGuid());
#endif

    SceneMutator mut(p_scene);
    cb.Playback(mut);
    return cb.Resolve(e);
}

static Entity CreateMeshEntity(const std::string& p_asset_path,
                               Scene& p_scene,
                               std::string_view p_name) {
    Entity id = EntityFactory::CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(id);

    MeshRendererComponent& renderer = p_scene.Create<MeshRendererComponent>(id);

    auto mat_id = EntityFactory::CreateNameEntity(p_scene, std::format("{}:mat", p_name));
    p_scene.Create<MaterialComponent>(mat_id);
    renderer.AddMaterial(mat_id);

    // @TODO: create material
    auto handle = AssetRegistry::GetSingleton().FindByPath<MeshAsset>(p_asset_path).unwrap();
    renderer.SetResourceGuid(handle.GetGuid());
    return id;
}

Entity EntityFactory::CreatePlaneEntity(Scene& p_scene, std::string_view p_name) {
    return CreateMeshEntity("@persist://meshes/plane", p_scene, p_name);
}

Entity EntityFactory::CreateCubeEntity(Scene& p_scene, std::string_view p_name) {
    return CreateMeshEntity("@persist://meshes/cube", p_scene, p_name);
}

Entity EntityFactory::CreateSphereEntity(Scene& p_scene, std::string_view p_name) {
    return CreateMeshEntity("@persist://meshes/sphere", p_scene, p_name);
}

Entity EntityFactory::CreateCylinderEntity(Scene& p_scene, std::string_view p_name) {
    return CreateMeshEntity("@persist://meshes/cylinder", p_scene, p_name);
}

Entity EntityFactory::CreateConeEntity(Scene& p_scene, std::string_view p_name) {
    return CreateMeshEntity("@persist://meshes/cone", p_scene, p_name);
}

Entity EntityFactory::CreateTorusEntity(Scene& p_scene, std::string_view p_name) {
    return CreateMeshEntity("@persist://meshes/torus", p_scene, p_name);
}

Entity EntityFactory::CreateTileMapEntity(Scene& p_scene, std::string_view p_name) {
    Entity entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(entity);
    p_scene.Create<TileMapRendererComponent>(entity);
    return entity;
}

}  // namespace cave
