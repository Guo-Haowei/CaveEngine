#include "entity_factory.h"

#include "engine/assets/material_asset.h"
#include "engine/math/geometry.h"
#include "engine/runtime/asset_manager.h"
#include "engine/runtime/asset_registry.h"

// @TODO: refactor
#pragma warning(push)
#pragma warning(disable : 4100)  // unreferenced formal parameter
#pragma warning(disable : 4189)  // local variable is initialized but not referenced

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
    camera.SetDirty();
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
    auto entity = CreateObjectEntity(p_scene, p_name);

    LightComponent& light = p_scene.Create<LightComponent>(entity);
    light.SetType(LIGHT_TYPE_POINT);
    light.m_atten_constant = 1.0f;
    light.m_atten_linear = 0.2f;
    light.m_atten_quadratic = 0.05f;

    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(entity);
    MeshRendererComponent& object = *p_scene.GetComponent<MeshRendererComponent>(entity);
    transform.SetTranslation(p_position);
    transform.SetDirty();

    CRASH_NOW();
    unused(p_color);
    unused(p_emissive);
#if 0
    MaterialComponent& material = p_scene.Create<MaterialComponent>(entity);
    material.baseColor = Vector4f(p_color, 1.0f);
    material.emissive = p_emissive;

    auto mesh_id = CreateMeshEntity(p_scene, p_name + ":mesh");
    //object.meshId = mesh_id;
    object.flags = MeshRenderer::FLAG_RENDERABLE;

    MeshAsset& mesh = *p_scene.GetComponent<MeshAsset>(mesh_id);
    mesh = MakeSphereMesh(0.1f, 40, 40);
    mesh.subsets[0].material_id = Guid();
#endif
    return entity;
}

Entity EntityFactory::CreateAreaLightEntity(Scene& p_scene,
                                            const std::string& p_name,
                                            const Vector3f& p_color,
                                            const float p_emissive) {
    auto res = AssetManager::GetSingleton().CreateAsset(AssetType::Material,
                                                        std::format("@res://materials/{}.mat", p_name));
    if (!res) {
        CRASH_NOW();  // @TODO: error handling
    }
    Guid guid = res.value();

    Handle<MaterialAsset> mat_handle = AssetRegistry::GetSingleton().FindByGuid<MaterialAsset>(guid).unwrap();
    const std::shared_ptr<MaterialAsset>& mat = mat_handle.Wait();
    mat->base_color = Vector4f(p_color, 1.0f);
    mat->emissive = p_emissive;

    auto entity = CreateObjectEntity(p_scene, p_name);

    // light
    LightComponent& light = p_scene.Create<LightComponent>(entity);
    light.SetType(LIGHT_TYPE_AREA);
    light.m_base_color = mat->base_color;
    light.m_emissive = p_emissive;

    light.m_atten_constant = 1.0f;
    light.m_atten_linear = 0.09f;
    light.m_atten_quadratic = 0.032f;

    CRASH_NOW();

#if 0
    MeshRenderer& object = *p_scene.GetComponent<MeshRenderer>(entity);

    auto mesh_id = CreateMeshEntity(p_scene, p_name + ":mesh");
    object.meshId = mesh_id;
    object.flags = MeshRenderer::FLAG_RENDERABLE;

    MeshAsset& mesh = *p_scene.GetComponent<MeshAsset>(mesh_id);
    mesh = MakePlaneMesh();
    mesh.subsets[0].material_id = mat_handle.GetGuid();
    mesh.subsets[0].material_handle = mat_handle;
#endif
    return entity;
}

Entity EntityFactory::CreateInfiniteLightEntity(Scene& p_scene,
                                                const std::string& p_name,
                                                const Vector3f& p_color,
                                                const float p_emissive) {
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(entity);
    LightComponent& light = p_scene.Create<LightComponent>(entity);
    light.SetType(LIGHT_TYPE_INFINITE);
    light.m_atten_constant = 1.0f;
    light.m_atten_linear = 0.0f;
    light.m_atten_quadratic = 0.0f;

    light.m_base_color = Vector4f(p_color, 1.0f);
    light.m_emissive = p_emissive;

    return entity;
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

Entity EntityFactory::CreatePlaneEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        const Vector3f& p_scale,
                                        const Matrix4x4f& p_transform) {
    Guid material_id;
    return CreatePlaneEntity(p_scene, p_name, material_id, p_scale, p_transform);
}

Entity EntityFactory::CreatePlaneEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        const Guid& p_material_id,
                                        const Vector3f& p_scale,
                                        const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_scene, p_name);
    TransformComponent& trans = *p_scene.GetComponent<TransformComponent>(entity);
    MeshRendererComponent& object = *p_scene.GetComponent<MeshRendererComponent>(entity);
    trans.MatrixTransform(p_transform);

#if 0
    auto mesh_id = CreateMeshEntity(p_scene, p_name + ":mesh");
    object.meshId = mesh_id;

    MeshAsset& mesh = *p_scene.GetComponent<MeshAsset>(mesh_id);
    mesh = MakePlaneMesh(p_scale);
    mesh.subsets[0].material_id = p_material_id;

#endif
    return entity;
}

Entity EntityFactory::CreateCubeEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       const Vector3f& p_scale,
                                       const Matrix4x4f& p_transform) {
    Guid material_id;
    return CreateCubeEntity(p_scene, p_name, material_id, p_scale, p_transform);
}

Entity EntityFactory::CreateCubeEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       const Guid& p_material_id,
                                       const Vector3f& p_scale,
                                       const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_scene, p_name);
    TransformComponent& trans = *p_scene.GetComponent<TransformComponent>(entity);
    MeshRendererComponent& object = *p_scene.GetComponent<MeshRendererComponent>(entity);
    trans.MatrixTransform(p_transform);

#if 0
    auto handle = CreateMeshEntity(p_scene, "@res://models/" + p_name + ".mesh");
    object.m_mesh_id = handle.GetGuid();
    object.m_mesh_handle = handle;

    std::shared_ptr<MeshAsset> mesh = handle.Wait();
    *mesh = MakeCubeMesh(p_scale);
    mesh->subsets[0].material_id = p_material_id;
    mesh->OnDeserialized();
#endif

    return entity;
}

Entity EntityFactory::CreateSphereEntity(Scene& p_scene,
                                         const std::string& p_name,
                                         float p_radius,
                                         const Matrix4x4f& p_transform) {
    Guid material_id;
    return CreateSphereEntity(p_scene, p_name, material_id, p_radius, p_transform);
}

Entity EntityFactory::CreateSphereEntity(Scene& p_scene,
                                         const std::string& p_name,
                                         const Guid& p_material_id,
                                         float p_radius,
                                         const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_scene, p_name);
    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

#if 0
    MeshRenderer& object = *p_scene.GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_scene, p_name + ":mesh");
    object.meshId = mesh_id;

    MeshAsset& mesh = *p_scene.GetComponent<MeshAsset>(mesh_id);
    mesh = MakeSphereMesh(p_radius);
    mesh.subsets[0].material_id = p_material_id;
#endif

    return entity;
}

Entity EntityFactory::CreateCylinderEntity(Scene& p_scene,
                                           const std::string& p_name,
                                           float p_radius,
                                           float p_height,
                                           const Matrix4x4f& p_transform) {
    Guid material_id;
    return CreateCylinderEntity(p_scene, p_name, material_id, p_radius, p_height, p_transform);
}

Entity EntityFactory::CreateCylinderEntity(Scene& p_scene,
                                           const std::string& p_name,
                                           const Guid& p_material_id,
                                           float p_radius,
                                           float p_height,
                                           const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_scene, p_name);
    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

#if 0
    MeshRenderer& object = *p_scene.GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_scene, p_name + ":mesh");
    object.meshId = mesh_id;

    MeshAsset& mesh = *p_scene.GetComponent<MeshAsset>(mesh_id);
    mesh = MakeCylinderMesh(p_radius, p_height);
    mesh.subsets[0].material_id = p_material_id;
#endif

    return entity;
}

Entity EntityFactory::CreateTorusEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        float p_radius,
                                        float p_tube_radius,
                                        const Matrix4x4f& p_transform) {
    Guid material_id;
    return CreateTorusEntity(p_scene, p_name, material_id, p_radius, p_tube_radius, p_transform);
}

Entity EntityFactory::CreateTorusEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        const Guid& p_material_id,
                                        float p_radius,
                                        float p_tube_radius,
                                        const Matrix4x4f& p_transform) {
    // @TODO: fix this
    p_radius = 0.4f;
    p_tube_radius = 0.1f;

    auto entity = CreateObjectEntity(p_scene, p_name);
    TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

#if 0
    MeshRenderer& object = *p_scene.GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_scene, p_name + ":mesh");
    object.meshId = mesh_id;

    MeshAsset& mesh = *p_scene.GetComponent<MeshAsset>(mesh_id);
    mesh = MakeTorusMesh(p_radius, p_tube_radius);
    mesh.subsets[0].material_id = p_material_id;
#endif

    return entity;
}

Entity EntityFactory::CreateTileMapEntity(Scene& p_scene,
                                          const std::string& p_name,
                                          const Matrix4x4f& p_transform) {
    auto entity = CreateNameEntity(p_scene, p_name);

    TransformComponent& transform = p_scene.Create<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    p_scene.Create<TileMapRendererComponent>(entity);
    return entity;
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
#endif

#if 0
Entity EntityFactory::CreateMeshEmitterEntity(Scene& p_scene,
                                              const std::string& p_name,
                                              const Vector3f& p_translation) {
    LOG_WARN("TODO: fix");
    auto entity = CreateNameEntity(p_scene, p_name);
    p_scene.Create<TransformComponent>(entity).SetTranslation(p_translation);
    p_scene.Create<MeshEmitterComponent>(entity);
    return entity;
}
#endif

#if 0
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

}  // namespace cave
