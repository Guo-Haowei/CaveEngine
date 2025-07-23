#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/ecs/entity.h"
#include "engine/reflection/reflection.h"

namespace cave {

struct MeshRendererComponent {
    CAVE_META(MeshRendererComponent)

    CAVE_PROP(editor = Asset)
    Guid m_mesh_id;

    CAVE_PROP(editor = Visibility)
    bool m_is_visible = true;

    CAVE_PROP(editor = Toggle)
    bool m_cast_shadow = true;

    CAVE_PROP(editor = Toggle)
    bool m_transparency = false;

    CAVE_PROP()
    std::vector<Guid> m_material_ids;

    // Non-serialized
    Handle<MeshAsset> m_mesh_handle{};
    std::vector<Handle<MaterialAsset>> m_material_handles{};

public:
    MeshRendererComponent();

    const Guid& GetResourceGuid() const { return m_mesh_id; }
    void SetResourceGuid(const Guid& p_guid);

    auto& GetMaterialGuids() { return m_material_ids; }
    const auto& GetMaterialGuids() const { return m_material_ids; }

    auto& GetMaterialHandles() { return m_material_handles; }
    const auto& GetMaterialHandles() const { return m_material_handles; }

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
