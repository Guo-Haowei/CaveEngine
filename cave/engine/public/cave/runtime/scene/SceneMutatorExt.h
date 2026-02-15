// =============================================================================
// File: public/cave/runtime/scene/SceneMutatorExt.h
// =============================================================================
#pragma once
#include <string_view>
#include "cave/core/math/Vector.h"
#include "cave/core/ids/Guid.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneMutator.h"

namespace cave {

class AssetRegistry;

class SceneExt {
    using Entity = ecs::Entity;
    using Vector3f = math::Vector3f;
    using Vector4f = math::Vector4f;

public:
    explicit SceneExt(AssetRegistry& p_reg) noexcept
        : m_asset_reg(p_reg) {
    }

    static Entity FindEntityByName(const Scene& p_scene, std::string_view p_name);

    Entity CreateRootObject(SceneCommandBuffer& p_cb, std::string_view p_name = "root");
    Entity CreateNameObject(SceneCommandBuffer& p_cb, std::string_view p_name);

    Entity CreateTransformObject(SceneCommandBuffer& p_cb, std::string_view p_name);

    void AttachChild(SceneCommandBuffer& p_cb, Entity p_child, Entity p_parent);

    Entity CreatePointLightObject(SceneCommandBuffer& p_cb,
                                  std::string_view p_name,
                                  const Vector3f& p_position = Vector3f(0.0f, 1.0f, 0.0f),
                                  const Vector3f& p_color = Vector3f(1.0f),
                                  float p_emissive = 5.0f);

    Entity CreateAreaLightObject(SceneCommandBuffer& p_cb,
                                 std::string_view p_name,
                                 const Vector3f& p_color = Vector3f(1),
                                 float p_emissive = 5.0f);

    Entity CreateInfiniteLightObject(SceneCommandBuffer& p_cb,
                                     std::string_view p_name,
                                     const Vector3f& p_color = Vector3f(1),
                                     float p_emissive = 5.0f);

    Entity CreateMeshObject(const std::string& p_asset_path,
                            SceneCommandBuffer& p_cb,
                            std::string_view p_name,
                            const Guid* p_mat_guid);

    Entity CreatePlaneObject(SceneCommandBuffer& p_cb,
                             std::string_view p_name,
                             const Guid* p_mat_id = nullptr);

    Entity CreateCubeObject(SceneCommandBuffer& p_cb,
                            std::string_view p_name,
                            const Guid* p_mat_guid = nullptr);

    Entity CreateSphereObject(SceneCommandBuffer& p_cb,
                              std::string_view p_name,
                              const Guid* p_mat_guid = nullptr);

    Entity CreateCylinderObject(SceneCommandBuffer& p_cb,
                                std::string_view p_name,
                                const Guid* p_mat_id = nullptr);

    Entity CreateConeObject(SceneCommandBuffer& p_cb,
                            std::string_view p_name,
                            const Guid* p_mat_id = nullptr);

    Entity CreateTorusObject(SceneCommandBuffer& p_cb,
                             std::string_view p_name,
                             const Guid* p_mat_id = nullptr);

    Entity CreateTileMapObject(SceneCommandBuffer& p_cb, std::string_view p_name);

private:
    AssetRegistry& m_asset_reg;
};

}  // namespace cave
