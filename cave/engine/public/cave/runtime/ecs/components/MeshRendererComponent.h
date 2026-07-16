// =============================================================================
// File: cave/runtime/ecs/components/MeshRendererComponent.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedStack.h"
#include "cave/core/ids/Entity.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class MeshRendererComponent {
    CAVE_COMPONENT(MeshRendererComponent)

    static constexpr int kMaxMaterial = 8;

private:
    CAVE_PROP(editor = Toggle)
    bool m_visibility = true;

    CAVE_PROP(editor = Toggle)
    bool m_cast_shadow = true;

    CAVE_PROP(editor = Toggle)
    bool m_transparency = false;

    CAVE_PROP(editor = Asset, on_change = onMeshGuidChanged)
    Guid m_mesh_id;

    CAVE_PROP()
    FixedStack<ecs::Entity, kMaxMaterial> m_materials;

    CAVE_PROP()
    ecs::Entity m_skeleton_id;

    // Non-serialized
    Handle<MeshAsset> m_mesh_handle{};

    void refreshMeshHandle();
    void onMeshGuidChanged(const FieldChange& change);

public:
    MeshRendererComponent();

    const Guid& meshGuid() const { return m_mesh_id; }
    void setMeshGuid(const Guid& guid);

    auto& materialInstances() { return m_materials; }
    const auto& materialInstances() const { return m_materials; }

    void addMaterial(ecs::Entity material);

    const auto& meshHandle() const { return m_mesh_handle; }

    ecs::Entity GetSkeletonId() const { return m_skeleton_id; }
    void SetSkeletonId(ecs::Entity p_id) { m_skeleton_id = p_id; }

    void SetVisible(bool p_value = true) { m_visibility = p_value; }
    bool IsVisible() const { return m_visibility; }

    void SetCastShadow(bool p_value = true) { m_cast_shadow = p_value; }
    bool CastShadow() const { return m_cast_shadow; }

    void SetTransparency(bool p_value = true) { m_transparency = p_value; }
    bool Transparency() const { return m_transparency; }

    void onDeserialized();
};

}  // namespace cave
