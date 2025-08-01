#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/ecs/entity.h"
#include "engine/reflection/reflection.h"

namespace cave {

class MeshRendererComponent {
    CAVE_META(MeshRendererComponent)

private:
    CAVE_PROP(editor = Visibility)
    bool m_is_visible = true;

    CAVE_PROP(editor = Toggle)
    bool m_cast_shadow = true;

    CAVE_PROP(editor = Toggle)
    bool m_transparency = false;

    CAVE_PROP(editor = Asset)
    Guid m_mesh_id;

    CAVE_PROP()
    std::vector<ecs::Entity> m_materials;

    // Non-serialized
    Handle<MeshAsset> m_mesh_handle{};

public:
    MeshRendererComponent();

    const Guid& GetResourceGuid() const { return m_mesh_id; }
    void SetResourceGuid(const Guid& p_guid);

    auto& GetMaterialInstances() { return m_materials; }
    const auto& GetMaterialInstances() const { return m_materials; }

    void AddMaterial(ecs::Entity& p_material);

    const auto& GetMeshHandle() const { return m_mesh_handle; }

    void SetVisible(bool p_value = true) { m_is_visible = p_value; }
    bool IsVisible() const { return m_is_visible; }

    void SetCastShadow(bool p_value = true) { m_cast_shadow = p_value; }
    bool CastShadow() const { return m_cast_shadow; }

    void SetTransparency(bool p_value = true) { m_transparency = p_value; }
    bool Transparency() const { return m_transparency; }

    void OnDeserialized();
};

}  // namespace cave
