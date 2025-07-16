#include "entity_factory.h"

#include "engine/math/geometry.h"
#include "engine/scene/scene.h"

namespace cave {

ecs::Entity Scene::CreatePerspectiveCameraEntity(const std::string& p_name,
                                                 int p_width,
                                                 int p_height,
                                                 float p_near_plane,
                                                 float p_far_plane,
                                                 Degree p_fovy) {
    auto entity = CreateNameEntity(p_name);
    CameraComponent& camera = Create<CameraComponent>(entity);

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

ecs::Entity Scene::CreateNameEntity(const std::string& p_name) {
    auto entity = ecs::Entity::Create();
    Create<NameComponent>(entity).SetName(p_name);
    return entity;
}

ecs::Entity Scene::CreateTransformEntity(const std::string& p_name) {
    auto entity = CreateNameEntity(p_name);
    Create<TransformComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreateObjectEntity(const std::string& p_name) {
    auto entity = CreateNameEntity(p_name);
    Create<MeshRenderer>(entity);
    Create<TransformComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreateMeshEntity(const std::string& p_name) {
    auto entity = CreateNameEntity(p_name);
    Create<MeshComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreateMaterialEntity(const std::string& p_name) {
    auto entity = CreateNameEntity(p_name);
    Create<MaterialComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreatePointLightEntity(const std::string& p_name,
                                          const Vector3f& p_position,
                                          const Vector3f& p_color,
                                          const float p_emissive) {
    auto entity = CreateObjectEntity(p_name);

    LightComponent& light = Create<LightComponent>(entity);
    light.SetType(LIGHT_TYPE_POINT);
    light.m_atten.constant = 1.0f;
    light.m_atten.linear = 0.2f;
    light.m_atten.quadratic = 0.05f;

    MaterialComponent& material = Create<MaterialComponent>(entity);
    material.baseColor = Vector4f(p_color, 1.0f);
    material.emissive = p_emissive;

    TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    transform.SetTranslation(p_position);
    transform.SetDirty();

    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;
    object.flags = MeshRenderer::FLAG_RENDERABLE;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakeSphereMesh(0.1f, 40, 40);
    mesh.subsets[0].material_id = entity;
    return entity;
}

ecs::Entity Scene::CreateAreaLightEntity(const std::string& p_name,
                                         const Vector3f& p_color,
                                         const float p_emissive) {
    auto entity = CreateObjectEntity(p_name);

    // light
    LightComponent& light = Create<LightComponent>(entity);
    light.SetType(LIGHT_TYPE_AREA);
    // light.m_atten.constant = 1.0f;
    // light.m_atten.linear = 0.09f;
    // light.m_atten.quadratic = 0.032f;

    // material
    MaterialComponent& material = Create<MaterialComponent>(entity);
    material.baseColor = Vector4f(p_color, 1.0f);
    material.emissive = p_emissive;

    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);

    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;
    object.flags = MeshRenderer::FLAG_RENDERABLE;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakePlaneMesh();
    mesh.subsets[0].material_id = entity;
    return entity;
}

ecs::Entity Scene::CreateInfiniteLightEntity(const std::string& p_name,
                                             const Vector3f& p_color,
                                             const float p_emissive) {
    auto entity = CreateNameEntity(p_name);

    Create<TransformComponent>(entity);

    LightComponent& light = Create<LightComponent>(entity);
    light.SetType(LIGHT_TYPE_INFINITE);
    light.m_atten.constant = 1.0f;
    light.m_atten.linear = 0.0f;
    light.m_atten.quadratic = 0.0f;

    MaterialComponent& material = Create<MaterialComponent>(entity);
    material.baseColor = Vector4f(p_color, 1.0f);
    material.emissive = p_emissive;
    return entity;
}

ecs::Entity Scene::CreateEnvironmentEntity(const std::string& p_name) {
    auto entity = CreateNameEntity(p_name);
    Create<EnvironmentComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreateVoxelGiEntity(const std::string& p_name) {
    auto entity = CreateNameEntity(p_name);
    Create<VoxelGiComponent>(entity);
    Create<TransformComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreatePlaneEntity(const std::string& p_name,
                                     const Vector3f& p_scale,
                                     const Matrix4x4f& p_transform) {
    auto material_id = CreateMaterialEntity(p_name + ":mat");
    return CreatePlaneEntity(p_name, material_id, p_scale, p_transform);
}

ecs::Entity Scene::CreatePlaneEntity(const std::string& p_name,
                                     ecs::Entity p_material_id,
                                     const Vector3f& p_scale,
                                     const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_name);
    TransformComponent& trans = *GetComponent<TransformComponent>(entity);
    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    trans.MatrixTransform(p_transform);

    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakePlaneMesh(p_scale);
    mesh.subsets[0].material_id = p_material_id;

    return entity;
}

ecs::Entity Scene::CreateCubeEntity(const std::string& p_name,
                                    const Vector3f& p_scale,
                                    const Matrix4x4f& p_transform) {
    auto material_id = CreateMaterialEntity(p_name + ":mat");
    return CreateCubeEntity(p_name, material_id, p_scale, p_transform);
}

ecs::Entity Scene::CreateCubeEntity(const std::string& p_name,
                                    ecs::Entity p_material_id,
                                    const Vector3f& p_scale,
                                    const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_name);
    TransformComponent& trans = *GetComponent<TransformComponent>(entity);
    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    trans.MatrixTransform(p_transform);

    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakeCubeMesh(p_scale);
    mesh.subsets[0].material_id = p_material_id;

    return entity;
}

ecs::Entity Scene::CreateMeshEntity(const std::string& p_name,
                                    ecs::Entity p_material_id,
                                    MeshComponent&& p_mesh) {
    auto entity = CreateObjectEntity(p_name);
    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);

    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = std::move(p_mesh);
    mesh.subsets[0].material_id = p_material_id;
    return entity;
}

ecs::Entity Scene::CreateSphereEntity(const std::string& p_name,
                                      float p_radius,
                                      const Matrix4x4f& p_transform) {
    auto material_id = CreateMaterialEntity(p_name + ":mat");
    return CreateSphereEntity(p_name, material_id, p_radius, p_transform);
}

ecs::Entity Scene::CreateSphereEntity(const std::string& p_name,
                                      ecs::Entity p_material_id,
                                      float p_radius,
                                      const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_name);
    TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakeSphereMesh(p_radius);
    mesh.subsets[0].material_id = p_material_id;

    return entity;
}

ecs::Entity Scene::CreateCylinderEntity(const std::string& p_name,
                                        float p_radius,
                                        float p_height,
                                        const Matrix4x4f& p_transform) {
    auto material_id = CreateMaterialEntity(p_name + ":mat");
    return CreateCylinderEntity(p_name, material_id, p_radius, p_height, p_transform);
}

ecs::Entity Scene::CreateCylinderEntity(const std::string& p_name,
                                        ecs::Entity p_material_id,
                                        float p_radius,
                                        float p_height,
                                        const Matrix4x4f& p_transform) {
    auto entity = CreateObjectEntity(p_name);
    TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakeCylinderMesh(p_radius, p_height);
    mesh.subsets[0].material_id = p_material_id;

    return entity;
}

ecs::Entity Scene::CreateTorusEntity(const std::string& p_name,
                                     float p_radius,
                                     float p_tube_radius,
                                     const Matrix4x4f& p_transform) {
    auto material_id = CreateMaterialEntity(p_name + ":mat");
    return CreateTorusEntity(p_name, material_id, p_radius, p_tube_radius, p_transform);
}

ecs::Entity Scene::CreateTorusEntity(const std::string& p_name,
                                     ecs::Entity p_material_id,
                                     float p_radius,
                                     float p_tube_radius,
                                     const Matrix4x4f& p_transform) {
    // @TODO: fix this
    p_radius = 0.4f;
    p_tube_radius = 0.1f;

    auto entity = CreateObjectEntity(p_name);
    TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakeTorusMesh(p_radius, p_tube_radius);
    mesh.subsets[0].material_id = p_material_id;

    return entity;
}

ecs::Entity Scene::CreateTileMapEntity(const std::string& p_name, const Matrix4x4f& p_transform) {
    auto entity = CreateNameEntity(p_name);

    TransformComponent& transform = Create<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    Create<TileMapRenderer>(entity);
    return entity;
}

ecs::Entity Scene::CreateClothEntity(const std::string& p_name,
                                     ecs::Entity p_material_id,
                                     const Vector3f& p_point_0,
                                     const Vector3f& p_point_1,
                                     const Vector3f& p_point_2,
                                     const Vector3f& p_point_3,
                                     const Vector2i& p_res,
                                     ClothFixFlag p_fixed_flags,
                                     const Matrix4x4f& p_transform) {
    // @TODO: fix
    unused(p_transform);

    auto entity = CreateObjectEntity(p_name);
    // TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    // transform.MatrixTransform(p_transform);

    ClothComponent& cloth = Create<ClothComponent>(entity);
    cloth.point_0 = p_point_0;
    cloth.point_1 = p_point_1;
    cloth.point_2 = p_point_2;
    cloth.point_3 = p_point_3;
    cloth.res = p_res;
    cloth.fixedFlags = p_fixed_flags;

    MeshRenderer& object = *GetComponent<MeshRenderer>(entity);
    auto mesh_id = CreateMeshEntity(p_name + ":mesh");
    object.meshId = mesh_id;

    MeshComponent& mesh = *GetComponent<MeshComponent>(mesh_id);
    mesh = MakePlaneMesh(p_point_0,
                         p_point_1,
                         p_point_2,
                         p_point_3);
    mesh.subsets[0].material_id = p_material_id;

    return entity;
}

ecs::Entity Scene::CreateEmitterEntity(const std::string& p_name, const Matrix4x4f& p_transform) {
    auto entity = CreateTransformEntity(p_name);
    Create<ParticleEmitterComponent>(entity);

    TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);
    return entity;
}

ecs::Entity Scene::CreateMeshEmitterEntity(const std::string& p_name, const Vector3f& p_translation) {
    auto entity = CreateNameEntity(p_name);
    Create<TransformComponent>(entity).SetTranslation(p_translation);
    Create<MeshEmitterComponent>(entity);
    return entity;
}

ecs::Entity Scene::CreateForceFieldEntity(const std::string& p_name, const Matrix4x4f& p_transform) {
    auto entity = CreateTransformEntity(p_name);
    Create<ForceFieldComponent>(entity);

    TransformComponent& transform = *GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(p_transform);

    return entity;
}

}  // namespace cave
