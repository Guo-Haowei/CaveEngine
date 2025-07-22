#pragma once
#include "engine/scene/scene.h"

namespace cave {
using ecs::Entity;

class EntityFactory {
public:
    static Entity CreatePerspectiveCameraEntity(Scene& p_scene,
                                                const std::string& p_name,
                                                int p_width,
                                                int p_height,
                                                float p_near_plane = CameraComponent::DEFAULT_NEAR,
                                                float p_far_plane = CameraComponent::DEFAULT_FAR,
                                                Degree p_fovy = CameraComponent::DEFAULT_FOVY);

    static Entity CreateNameEntity(Scene& p_scene,
                                   const std::string& p_name);

    static Entity CreateTransformEntity(Scene& p_scene,
                                        const std::string& p_name);

    static Entity CreateObjectEntity(Scene& p_scene,
                                     const std::string& p_name);

    static Entity CreatePointLightEntity(Scene& p_scene,
                                         const std::string& p_name,
                                         const Vector3f& p_position = Vector3f(0.0f, 1.0f, 0.0f),
                                         const Vector3f& p_color = Vector3f(1.0f),
                                         const float p_emissive = 5.0f);

    static Entity CreateAreaLightEntity(Scene& p_scene,
                                        const std::string& p_name,
                                        const Vector3f& p_color = Vector3f(1),
                                        const float p_emissive = 5.0f);

    static Entity CreateInfiniteLightEntity(Scene& p_scene,
                                            const std::string& p_name,
                                            const Vector3f& p_color = Vector3f(1),
                                            const float p_emissive = 5.0f);

    static Entity CreateEnvironmentEntity(Scene& p_scene,
                                          const std::string& p_name);

    static Entity CreateVoxelGiEntity(Scene& p_scene,
                                      const std::string& p_name);

    static Entity CreatePlaneEntity(Scene& p_scene,
                                    const std::string& p_name,
                                    const Vector3f& p_scale = Vector3f(0.5f),
                                    const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreatePlaneEntity(Scene& p_scene,
                                    const std::string& p_name,
                                    const Guid& p_material_id,
                                    const Vector3f& p_scale = Vector3f(0.5f),
                                    const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateCubeEntity(Scene& p_scene,
                                   const std::string& p_name,
                                   const Vector3f& p_scale = Vector3f(0.5f),
                                   const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateCubeEntity(Scene& p_scene,
                                   const std::string& p_name,
                                   const Guid& p_material_id,
                                   const Vector3f& p_scale = Vector3f(0.5f),
                                   const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateSphereEntity(Scene& p_scene,
                                     const std::string& p_name,
                                     float p_radius = 0.5f,
                                     const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateSphereEntity(Scene& p_scene,
                                     const std::string& p_name,
                                     const Guid& p_material_id,
                                     float p_radius = 0.5f,
                                     const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateCylinderEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       float p_radius = 0.5f,
                                       float p_height = 1.0f,
                                       const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateCylinderEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       const Guid& p_material_id,
                                       float p_radius = 0.5f,
                                       float p_height = 1.0f,
                                       const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateTorusEntity(Scene& p_scene,
                                    const std::string& p_name,
                                    float p_radius = 0.5f,
                                    float p_tube_radius = 0.2f,
                                    const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateTorusEntity(Scene& p_scene,
                                    const std::string& p_name,
                                    const Guid& p_material_id,
                                    float p_radius = 0.5f,
                                    float p_tube_radius = 0.2f,
                                    const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

#if 0
    static Entity CreateEmitterEntity(Scene& p_scene,
                                      const std::string& p_name,
                                      const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateMeshEmitterEntity(Scene& p_scene,
                                          const std::string& p_name,
                                          const Vector3f& p_translation = Vector3f::Zero);

    static Entity CreateForceFieldEntity(Scene& p_scene,
                                         const std::string& p_name,
                                         const Matrix4x4f& p_transform = Matrix4x4f(1.0f));
#endif

    static Entity CreateTileMapEntity(Scene& p_scene,
                                      const std::string& p_name,
                                      const Matrix4x4f& p_transform = Matrix4x4f(1.0f));
};

}  // namespace cave
