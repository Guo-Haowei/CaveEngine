#include "entity_factory.h"

#include "engine/assets/material_asset.h"
#include "engine/math/geometry.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

Entity EntityFactory::CreatePerspectiveCameraEntity(Scene& p_scene,
                                                    const std::string& p_name,
                                                    int p_width,
                                                    int p_height,
                                                    float p_near_plane,
                                                    float p_far_plane,
                                                    Degree p_fovy) {
    auto entity = CreateNameEntity(p_scene, p_name);
    CameraComponent& camera = p_scene.Create<CameraComponent>(entity);

    camera.m_width = p_width;
    camera.m_height = p_height;
    camera.m_near = p_near_plane;
    camera.m_far = p_far_plane;
    camera.m_fovy = p_fovy;
    camera.m_pitch = Degree{ -10.0f };
    camera.m_yaw = Degree{ -90.0f };
    camera.SetDirtyFlag();
    return entity;
}

Entity EntityFactory::CreateNameEntity(Scene& p_scene,
                                       const std::string& p_name) {
    auto entity = p_scene.CreateEntity();
    p_scene.Create<NameComponent>(entity).SetName(p_name);
    return entity;
}

Entity EntityFactory::CreateTransformEntity(Scene& p_scene,
                                            const std::string& p_name) {
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(entity);
    return entity;
}

Entity EntityFactory::CreateObjectEntity(Scene& p_scene,
                                         const std::string& p_name) {
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<MeshRendererComponent>(entity);
    p_scene.Create<TransformComponent>(entity);
    return entity;
}

Entity EntityFactory::CreatePointLightEntity(Scene& p_scene,
                                             const std::string& p_name,
                                             const Vector3f& p_position,
                                             const Vector3f& p_color,
                                             const float p_emissive) {
    auto id = CreateObjectEntity(p_scene, p_name);

    LightComponent& light = p_scene.Create<LightComponent>(id);
    light.SetType(LightType::Point);
    light.m_atten_constant = 1.0f;
    light.m_atten_linear = 0.2f;
    light.m_atten_quadratic = 0.05f;

    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(id);
    transform.SetTranslation(p_position);
    transform.SetDirty();

    MaterialComponent& mat = p_scene.Create<MaterialComponent>(id);
    mat.base_color = Vector4f(p_color, 1.0f);
    mat.emissive = p_emissive;

    return id;
}

Entity EntityFactory::CreateInfiniteLightEntity(Scene& p_scene,
                                                const std::string& p_name,
                                                const Vector3f& p_color,
                                                const float p_emissive) {
    auto id = CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(id);
    LightComponent& light = p_scene.Create<LightComponent>(id);
    light.SetType(LightType::Infinite);
    light.m_atten_constant = 1.0f;
    light.m_atten_linear = 0.0f;
    light.m_atten_quadratic = 0.0f;

    MaterialComponent& mat = p_scene.Create<MaterialComponent>(id);
    mat.base_color = Vector4f(p_color, 1.0f);
    mat.emissive = p_emissive;

    return id;
}

Entity EntityFactory::CreateAreaLightEntity(Scene& p_scene,
                                            const std::string& p_name,
                                            const Vector3f& p_color,
                                            const float p_emissive) {
    auto id = CreateObjectEntity(p_scene, p_name);

    // light
    LightComponent& light = p_scene.Create<LightComponent>(id);
    light.SetType(LightType::Area);

    light.m_atten_constant = 1.0f;
    light.m_atten_linear = 0.09f;
    light.m_atten_quadratic = 0.032f;

    // material
    MaterialComponent& mat = p_scene.Create<MaterialComponent>(id);
    mat.base_color = Vector4f(p_color, 1.0f);
    mat.emissive = p_emissive;

    // mesh
    MeshRendererComponent& renderer = *p_scene.GetComponent<MeshRendererComponent>(id);
    renderer.AddMaterial(id);

    auto handle = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/plane").unwrap();
    renderer.SetResourceGuid(handle.GetGuid());
    return id;
}

Entity EntityFactory::CreateEnvironmentEntity(Scene& p_scene,
                                              const std::string& p_name) {
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<EnvironmentComponent>(entity);
    return entity;
}

Entity EntityFactory::CreateVoxelGiEntity(Scene& p_scene,
                                          const std::string& p_name) {
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<VoxelGiComponent>(entity);
    p_scene.Create<TransformComponent>(entity);
    return entity;
}

static Entity CreateMeshEntity(const std::string& p_asset_path,
                               Scene& p_scene,
                               const std::string& p_name,
                               const Matrix4x4f& p_transform) {
    auto id = EntityFactory::CreateNameEntity(p_scene, p_name);
    TransformComponent& transform = p_scene.Create<TransformComponent>(id);
    transform.MatrixTransform(p_transform);

    MeshRendererComponent& renderer = p_scene.Create<MeshRendererComponent>(id);

    auto mat_id = EntityFactory::CreateNameEntity(p_scene, p_name + ":mat");
    p_scene.Create<MaterialComponent>(mat_id);
    renderer.AddMaterial(mat_id);

    // @TODO: create material
    auto handle = AssetRegistry::GetSingleton().FindByPath<MeshAsset>(p_asset_path).unwrap();
    renderer.SetResourceGuid(handle.GetGuid());
    return id;
}

Entity EntityFactory::CreatePlaneEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        const Matrix4x4f& p_transform) {
    return CreateMeshEntity("@persist://meshes/plane", p_scene, p_name, p_transform);
}

Entity EntityFactory::CreateCubeEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       const Matrix4x4f& p_transform) {
    return CreateMeshEntity("@persist://meshes/cube", p_scene, p_name, p_transform);
}

Entity EntityFactory::CreateSphereEntity(Scene& p_scene,
                                         const std::string& p_name,
                                         const Matrix4x4f& p_transform) {
    return CreateMeshEntity("@persist://meshes/sphere", p_scene, p_name, p_transform);
}

Entity EntityFactory::CreateCylinderEntity(Scene& p_scene,
                                           const std::string& p_name,
                                           const Matrix4x4f& p_transform) {
    return CreateMeshEntity("@persist://meshes/cylinder", p_scene, p_name, p_transform);
}

Entity EntityFactory::CreateConeEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       const Matrix4x4f& p_transform) {
    return CreateMeshEntity("@persist://meshes/cone", p_scene, p_name, p_transform);
}

Entity EntityFactory::CreateTorusEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        const Matrix4x4f& p_transform) {
    return CreateMeshEntity("@persist://meshes/torus", p_scene, p_name, p_transform);
}

#if 0

Entity EntityFactory::CreateEmitterEntity(Scene& p_scene,
                                          const std::string& p_name,
                                          const Matrix4x4f& p_transform) {
    LOG_WARN("TODO: fix");
    auto entity = CreateTransformEntity(p_scene, p_name);
    p_scene.Create<ParticleEmitterComponent>(entity);

    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);
    return entity;
}

Entity EntityFactory::CreateMeshEmitterEntity(Scene& p_scene,
                                              const std::string& p_name,
                                              const Vector3f& p_translation) {
    LOG_WARN("TODO: fix");
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(entity).SetTranslation(p_translation);
    p_scene.Create<MeshEmitterComponent>(entity);
    return entity;
}

Entity EntityFactory::CreateForceFieldEntity(Scene& p_scene,
                                             const std::string& p_name,
                                             const Matrix4x4f& p_transform) {
    LOG_WARN("TODO: fix");
    auto entity = CreateTransformEntity(p_scene, p_name);
    p_scene.Create<ForceFieldComponent>(entity);

    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);
    return entity;
}

#endif

Entity EntityFactory::CreateTileMapEntity(Scene& p_scene,
                                          const std::string& p_name,
                                          const Matrix4x4f& p_transform) {
    auto entity = CreateNameEntity(p_scene, p_name);

    TransformComponent& transform = p_scene.Create<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    p_scene.Create<TileMapRendererComponent>(entity);
    return entity;
}

}  // namespace cave
