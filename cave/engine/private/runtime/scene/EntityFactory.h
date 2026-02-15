#pragma once
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class EntityFactory {
    using Entity = cave::ecs::Entity;
    using Vector3f = cave::math::Vector3f;
    using Matrix4x4f = cave::math::Matrix4x4f;

public:
    static Entity CreateNameEntity(Scene& p_scene, std::string_view p_name);

    static Entity CreateTransformEntity(Scene& p_scene, std::string_view p_name);

    static Entity CreateMeshInstance(Scene& p_scene,
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

    static Entity CreatePlaneEntity(Scene& p_scene,
                                    const std::string& p_name,
                                    const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateCubeEntity(Scene& p_scene,
                                   const std::string& p_name,
                                   const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateSphereEntity(Scene& p_scene,
                                     const std::string& p_name,
                                     const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateCylinderEntity(Scene& p_scene,
                                       const std::string& p_name,
                                       const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateConeEntity(Scene& p_scene,
                                   const std::string& p_name,
                                   const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateTorusEntity(Scene& p_scene,
                                    const std::string& p_name,
                                    const Matrix4x4f& p_transform = Matrix4x4f(1.0f));

    static Entity CreateTileMapEntity(Scene& p_scene,
                                      const std::string& p_name,
                                      const Matrix4x4f& p_transform = Matrix4x4f(1.0f));
};

}  // namespace cave
