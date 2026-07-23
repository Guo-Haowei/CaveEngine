// =============================================================================
// File: cave/render/components/MeshRendererComponent.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedStack.h"
#include "cave/core/ids/Entity.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct FieldChange;

class MeshRendererComponent {
    CAVE_COMPONENT(MeshRendererComponent)

    static constexpr int kMaxMaterial = 8;

private:
    CAVE_PROP(editor = Toggle)
    bool m_visible = true;

    CAVE_PROP(editor = Toggle)
    bool m_cast_shadow = true;

    CAVE_PROP(editor = Toggle)
    bool m_transparent = false;

    CAVE_PROP(editor = Asset, on_change = onMeshGuidChanged)
    Guid m_mesh_guid;

    CAVE_PROP()
    FixedStack<ecs::Entity, 8> m_materials;

    CAVE_PROP()
    ecs::Entity m_skeleton_id;

    // Non-serialized
    Handle<MeshAsset> m_mesh_handle{};

    void refreshMeshHandle();
    void onMeshGuidChanged(const FieldChange& change);

public:
    MeshRendererComponent();

    const Guid& meshGuid() const { return m_mesh_guid; }
    void setMeshGuid(const Guid& guid);

    auto& materialInstances() { return m_materials; }
    const auto& materialInstances() const { return m_materials; }

    void addMaterial(ecs::Entity material);

    const auto& meshHandle() const { return m_mesh_handle; }

    ecs::Entity skeletonId() const { return m_skeleton_id; }
    void setSkeletonId(ecs::Entity id) { m_skeleton_id = id; }

    bool visible() const { return m_visible; }
    void setVisible(bool value) { m_visible = value; }

    bool castShadow() const { return m_cast_shadow; }
    void setCastShadow(bool value) { m_cast_shadow = value; }

    bool transparent() const { return m_transparent; }
    void setTransparent(bool value) { m_transparent = value; }

    void onDeserialized();
};

}  // namespace cave
