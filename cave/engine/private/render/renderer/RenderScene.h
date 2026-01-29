#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/AABB.h"
#include "cave/core/math/Matrix.h"

namespace cave {
struct GpuMesh;
struct GpuMaterial;
}  // namespace cave

namespace cave::render {

// Stable id for objects inside RenderScene (dense index / slot id).
using RenderObjectId = uint32_t;

// clang-format off
enum class RenderObjectFlags : uint32_t {
    None        = 0,
    CastShadow  = BIT(0),
    Transparent = BIT(1),
    Skinned     = BIT(2),
    Visible     = BIT(3),
    Highligted  = BIT(4),
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(RenderObjectFlags)

struct RenderMeshRef {
    const GpuMesh* mesh{ nullptr };
    uint32_t index_count{ 0 };

    bool IsValid() const { return mesh != nullptr && index_count != 0; }
};

struct RenderMaterialRef {
    uint32_t material_id{ 0 };
    bool IsValid() const { return material_id != 0; }
};

struct RenderObject {
    RenderObject() = default;

    ecs::Entity entity;
    RenderMeshRef mesh;
    RenderMaterialRef material;

    math::Matrix4x4f world;
    math::AABB world_aabb;

    ecs::Entity skeleton_entity;

    RenderObjectFlags flags{ RenderObjectFlags::None };

    // version stamps for incremental updates (component versions).
    // fill with 0 for now
    uint32_t transform_ver{ 0 };
    uint32_t mesh_ver{ 0 };
    uint32_t material_ver{ 0 };
    uint32_t skeleton_ver{ 0 };
};

// clang-format off
enum RenderDirtyFlags : uint32_t {
    RENGER_DIRTY_FLAG_NONE      = 0,
    RENGER_DIRTY_FLAG_TRANSFORM = BIT(0),
    RENGER_DIRTY_FLAG_MESH      = BIT(1),
    RENGER_DIRTY_FLAG_MATERIAL  = BIT(2),
    RENGER_DIRTY_FLAG_SKELETON  = BIT(3),
    RENGER_DIRTY_FLAG_ALL       = ~0u,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(RenderDirtyFlags);

class RenderScene {
public:
    RenderScene() = default;

    bool Contains(ecs::Entity p_entity) const {
        return m_entity_to_id.find(p_entity) != m_entity_to_id.end();
    }

    RenderObjectId GetId(ecs::Entity p_entity) const {
        auto it = m_entity_to_id.find(p_entity);
        return (it == m_entity_to_id.end()) ? kInvalidId : it->second;
    }

    RenderObject* TryGet(RenderObjectId p_id) {
        if (p_id >= m_objects.size()) return nullptr;
        return m_alive[p_id] ? &m_objects[p_id] : nullptr;
    }

    const RenderObject* TryGet(RenderObjectId p_id) const {
        if (p_id >= m_objects.size()) return nullptr;
        return m_alive[p_id] ? &m_objects[p_id] : nullptr;
    }

    RenderObjectId Ensure(ecs::Entity p_entity);

    void Remove(ecs::Entity p_entity);

    template<typename Fn>
    void ForEach(Fn&& p_fn) {
        for (RenderObjectId id = 0; id < (RenderObjectId)m_objects.size(); ++id) {
            if (!m_alive[id]) continue;
            p_fn(id, m_objects[id]);
        }
    }

    template<typename Fn>
    void ForEach(Fn&& p_fn) const {
        for (RenderObjectId id = 0; id < (RenderObjectId)m_objects.size(); ++id) {
            if (!m_alive[id]) continue;
            p_fn(id, m_objects[id]);
        }
    }

    // ---------- Dirty tracking ----------
    void MarkDirty(RenderObjectId p_id, RenderDirtyFlags p_dirty_flag);

    void ClearDirtyLists();

    const std::vector<RenderObjectId>& DirtyTransform() const { return m_dirty_transform; }
    const std::vector<RenderObjectId>& DirtyMesh() const { return m_dirty_mesh; }
    const std::vector<RenderObjectId>& DirtyMaterial() const { return m_dirty_material; }
    const std::vector<RenderObjectId>& DirtySkeleton() const { return m_dirty_skeleton; }

    // ---------- Access ----------
    const std::vector<RenderObject>& ObjectsUnsafe() const { return m_objects; }
    const std::vector<uint8_t>& AliveMaskUnsafe() const { return m_alive; }

    void Reset();

    static constexpr RenderObjectId kInvalidId = 0xFFFFFFFFu;

private:
    RenderObjectId AllocSlot();

    std::vector<RenderObject> m_objects;
    std::vector<uint8_t> m_alive;
    std::vector<RenderObjectId> m_free_ids;

    // ECS entity to render object slot id
    std::unordered_map<ecs::Entity, RenderObjectId> m_entity_to_id;

    // Dirty lists
    std::vector<RenderObjectId> m_dirty_transform;
    std::vector<RenderObjectId> m_dirty_mesh;
    std::vector<RenderObjectId> m_dirty_material;
    std::vector<RenderObjectId> m_dirty_skeleton;
};

}  // namespace cave::render
