// =============================================================================
// File: public/cave/runtime/scene/SceneCommandWriter.h
// =============================================================================
#pragma once
#include <string_view>
#include "cave/core/math/Vector.h"
#include "cave/core/ids/Guid.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"

namespace cave {

class AssetRegistry;

class SceneCommandWriter : public SceneCommandBuffer {
    using Entity = ecs::Entity;
    using Vector3f = math::Vector3f;
    using Vector4f = math::Vector4f;

public:
    explicit SceneCommandWriter(AssetRegistry& p_reg) noexcept
        : m_asset_reg(p_reg) {
    }

    // @NOTE: this should not be here, the command writer shouldn't know about scene
    static Entity FindEntityByName(const Scene& p_scene, std::string_view p_name);

    Entity CreateRootObject(std::string_view p_name = "root");
    Entity CreateNameObject(std::string_view p_name);

    Entity CreateTransformObject(std::string_view p_name);

    void AttachChild(Entity p_child, Entity p_parent);

    Entity CreatePointLightObject(std::string_view p_name,
                                  const Vector3f& p_position = Vector3f(0.0f, 1.0f, 0.0f),
                                  const Vector3f& p_color = Vector3f(1.0f),
                                  float p_emissive = 5.0f);

    Entity CreateAreaLightObject(std::string_view p_name,
                                 const Vector3f& p_color = Vector3f(1),
                                 float p_emissive = 5.0f);

    Entity CreateInfiniteLightObject(std::string_view p_name,
                                     const Vector3f& p_color = Vector3f(1),
                                     float p_emissive = 5.0f);

    Entity CreateMeshObject(const std::string& p_mesh_path,
                            std::string_view p_name,
                            const Guid* p_mat_guid);

    Entity CreateMeshObject(const std::string& p_mesh_path,
                            std::string_view p_name,
                            const std::string& p_mat_path);

    Entity CreatePlaneObject(std::string_view p_name,
                             const Guid* p_mat_id = nullptr);

    Entity CreateCubeObject(std::string_view p_name,
                            const Guid* p_mat_guid = nullptr);

    Entity CreateSphereObject(std::string_view p_name,
                              const Guid* p_mat_guid = nullptr);

    Entity CreateCylinderObject(std::string_view p_name,
                                const Guid* p_mat_id = nullptr);

    Entity CreateConeObject(std::string_view p_name,
                            const Guid* p_mat_id = nullptr);

    Entity CreateTorusObject(std::string_view p_name,
                             const Guid* p_mat_id = nullptr);

    Entity CreateTileMapObject(std::string_view p_name);

    void SetNoSave(bool p_value) { m_no_save = p_value; }

private:
    AssetRegistry& m_asset_reg;
    bool m_no_save{ false };
};

}  // namespace cave
