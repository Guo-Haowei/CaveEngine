// =============================================================================
// File: public/cave/runtime/scene/SceneMutatorExt.h
// =============================================================================
#pragma once
#include <string_view>
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"

namespace cave {

struct SceneExt {
    using Entity = ecs::Entity;
    using Vector3f = math::Vector3f;
    using Vector4f = math::Vector4f;

    static Entity CreateRootObject(SceneCommandBuffer& p_cb, std::string_view p_name = "root");
    static Entity CreateNameObject(SceneCommandBuffer& p_cb, std::string_view p_name);

    static Entity CreateTransformObject(SceneCommandBuffer& p_cb, std::string_view p_name);

    static void AttachChild(SceneCommandBuffer& p_cb, Entity p_child, Entity p_parent);

    static Entity CreatePointLightObject(SceneCommandBuffer& p_cb,
                                         std::string_view p_name,
                                         const Vector3f& p_position = Vector3f(0.0f, 1.0f, 0.0f),
                                         const Vector3f& p_color = Vector3f(1.0f),
                                         float p_emissive = 5.0f);

    static Entity CreateAreaLightObject(SceneCommandBuffer& p_cb,
                                        std::string_view p_name,
                                        const Vector3f& p_color = Vector3f(1),
                                        float p_emissive = 5.0f);

    static Entity CreateInfiniteLightObject(SceneCommandBuffer& p_cb,
                                            std::string_view p_name,
                                            const Vector3f& p_color = Vector3f(1),
                                            float p_emissive = 5.0f);

    static Entity CreatePlaneObject(SceneCommandBuffer& p_cb,
                                    std::string_view p_name,
                                    const Guid* p_mat_id = nullptr);

    static Entity CreateCubeObject(SceneCommandBuffer& p_cb,
                                   std::string_view p_name,
                                   const Guid* p_mat_guid = nullptr);

    static Entity CreateSphereObject(SceneCommandBuffer& p_cb,
                                     std::string_view p_name,
                                     const Guid* p_mat_guid = nullptr);

    static Entity CreateCylinderObject(SceneCommandBuffer& p_cb,
                                       std::string_view p_name,
                                       const Guid* p_mat_id = nullptr);

    static Entity CreateConeObject(SceneCommandBuffer& p_cb,
                                   std::string_view p_name,
                                   const Guid* p_mat_id = nullptr);

    static Entity CreateTorusObject(SceneCommandBuffer& p_cb,
                                    std::string_view p_name,
                                    const Guid* p_mat_id = nullptr);

    static Entity CreateTileMapObject(SceneCommandBuffer& p_cb, std::string_view p_name);
};

}  // namespace cave
