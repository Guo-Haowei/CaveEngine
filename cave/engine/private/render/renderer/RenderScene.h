#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "cave/core/ids/Entity.h"          // your ecs::Entity
#include "cave/core/math/AABB.h"
#include "cave/core/math/Matrix.h"

// Forward decls to avoid heavy includes in this header.
struct GpuMesh;
struct GpuMaterial; // if you have; otherwise keep MaterialId/handle.

namespace cave::render {

// Stable id for objects inside RenderScene (dense index / slot id).
using RenderObjectId = uint32_t;

enum class RenderObjectFlags : uint32_t {
    None         = 0,
    CastShadow   = 1u << 0,
    Transparent  = 1u << 1,
    Skinned      = 1u << 2,
    Visible      = 1u << 3, // optional, can be used for editor toggles
};

constexpr inline RenderObjectFlags operator|(RenderObjectFlags a, RenderObjectFlags b) {
    return static_cast<RenderObjectFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr inline RenderObjectFlags operator&(RenderObjectFlags a, RenderObjectFlags b) {
    return static_cast<RenderObjectFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr inline bool Any(RenderObjectFlags f) { return static_cast<uint32_t>(f) != 0; }

// What the renderer needs to draw a mesh right now (matches your FillPass usage)
struct RenderMeshRef {
    const GpuMesh* mesh{ nullptr };
    uint32_t index_count{ 0 };

    bool IsValid() const { return mesh != nullptr && index_count != 0; }
};

// Keep material reference abstract. You can start with an integer id or handle.
// Later this can become a pointer to compiled material/shader permutation, etc.
struct RenderMaterialRef {
    uint32_t material_id{ 0 }; // replace with your MaterialId type
    bool IsValid() const { return material_id != 0; }
};

// Per-object data in renderer-friendly form (no ECS iteration required later).
struct RenderObject {
    ecs::Entity entity;          // source entity (for updates/removal/debug)
    RenderMeshRef mesh;
    RenderMaterialRef material;

    math::Matrix4x4f world;      // for PerBatchConstantBuffer.c_worldMatrix
    math::AABB world_aabb;       // for culling

    ecs::Entity skeleton_entity; // used to fetch SkeletonComponent when needed

    RenderObjectFlags flags{ RenderObjectFlags::None };

    // Optional version stamps for incremental updates (component versions).
    // If you don’t have versions yet, keep them at 0 and fill later.
    uint32_t transform_ver{ 0 };
    uint32_t mesh_ver{ 0 };
    uint32_t material_ver{ 0 };
    uint32_t skeleton_ver{ 0 };
};

// Change tracking: what needs recompute in RenderScene → later stages.
enum class RenderDirty : uint8_t {
    Transform,
    Mesh,
    Material,
    Skeleton,
};

// A compact, renderer-friendly mirror of an ECS Scene.
class RenderScene {
public:
    RenderScene() = default;

    // ---------- Membership ----------
    bool Contains(ecs::Entity e) const {
        return m_entity_to_id.find(e) != m_entity_to_id.end();
    }

    RenderObjectId GetId(ecs::Entity e) const {
        auto it = m_entity_to_id.find(e);
        return (it == m_entity_to_id.end()) ? kInvalidId : it->second;
    }

    RenderObject* TryGet(RenderObjectId id) {
        if (id >= m_objects.size()) return nullptr;
        return m_alive[id] ? &m_objects[id] : nullptr;
    }
    const RenderObject* TryGet(RenderObjectId id) const {
        if (id >= m_objects.size()) return nullptr;
        return m_alive[id] ? &m_objects[id] : nullptr;
    }

    // Create or return existing id. Does NOT fill fields (builder will).
    RenderObjectId Ensure(ecs::Entity e) {
        auto it = m_entity_to_id.find(e);
        if (it != m_entity_to_id.end()) return it->second;

        RenderObjectId id = AllocSlot();
        m_entity_to_id.emplace(e, id);

        RenderObject& o = m_objects[id];
        o = RenderObject{};
        o.entity = e;

        MarkDirty(id, RenderDirty::Transform);
        MarkDirty(id, RenderDirty::Mesh);
        MarkDirty(id, RenderDirty::Material);
        MarkDirty(id, RenderDirty::Skeleton);
        return id;
    }

    void Remove(ecs::Entity e) {
        auto it = m_entity_to_id.find(e);
        if (it == m_entity_to_id.end()) return;

        const RenderObjectId id = it->second;
        m_entity_to_id.erase(it);

        if (id < m_objects.size() && m_alive[id]) {
            m_alive[id] = false;
            m_free_ids.push_back(id);
        }
    }

    // Dense iteration over alive objects
    template<typename Fn>
    void ForEach(Fn&& fn) {
        for (RenderObjectId id = 0; id < (RenderObjectId)m_objects.size(); ++id) {
            if (!m_alive[id]) continue;
            fn(id, m_objects[id]);
        }
    }

    template<typename Fn>
    void ForEach(Fn&& fn) const {
        for (RenderObjectId id = 0; id < (RenderObjectId)m_objects.size(); ++id) {
            if (!m_alive[id]) continue;
            fn(id, m_objects[id]);
        }
    }

    // ---------- Dirty tracking ----------
    void MarkDirty(RenderObjectId id, RenderDirty what) {
        if (id == kInvalidId) return;
        if (id >= m_objects.size() || !m_alive[id]) return;

        // Cheap de-dup is optional; start simple.
        switch (what) {
            case RenderDirty::Transform: m_dirty_transform.push_back(id); break;
            case RenderDirty::Mesh:      m_dirty_mesh.push_back(id); break;
            case RenderDirty::Material:  m_dirty_material.push_back(id); break;
            case RenderDirty::Skeleton:  m_dirty_skeleton.push_back(id); break;
            default: break;
        }
    }

    // After builder flushes updates, call this.
    void ClearDirtyLists() {
        m_dirty_transform.clear();
        m_dirty_mesh.clear();
        m_dirty_material.clear();
        m_dirty_skeleton.clear();
    }

    const std::vector<RenderObjectId>& DirtyTransform() const { return m_dirty_transform; }
    const std::vector<RenderObjectId>& DirtyMesh() const { return m_dirty_mesh; }
    const std::vector<RenderObjectId>& DirtyMaterial() const { return m_dirty_material; }
    const std::vector<RenderObjectId>& DirtySkeleton() const { return m_dirty_skeleton; }

    // ---------- Access ----------
    const std::vector<RenderObject>& ObjectsUnsafe() const { return m_objects; }
    const std::vector<uint8_t>& AliveMaskUnsafe() const { return m_alive; }

    void Reset() {
        m_objects.clear();
        m_alive.clear();
        m_free_ids.clear();
        m_entity_to_id.clear();
        ClearDirtyLists();
    }

    static constexpr RenderObjectId kInvalidId = 0xFFFFFFFFu;

private:
    RenderObjectId AllocSlot() {
        if (!m_free_ids.empty()) {
            const RenderObjectId id = m_free_ids.back();
            m_free_ids.pop_back();
            m_alive[id] = true;
            return id;
        }
        const RenderObjectId id = (RenderObjectId)m_objects.size();
        m_objects.emplace_back();
        m_alive.push_back(true);
        return id;
    }

private:
    // Slot storage (stable ids, supports remove without moving)
    std::vector<RenderObject> m_objects;
    std::vector<uint8_t> m_alive; // 1 byte mask per slot
    std::vector<RenderObjectId> m_free_ids;

    // ECS entity -> render object slot id
    std::unordered_map<ecs::Entity, RenderObjectId> m_entity_to_id;

    // Dirty lists
    std::vector<RenderObjectId> m_dirty_transform;
    std::vector<RenderObjectId> m_dirty_mesh;
    std::vector<RenderObjectId> m_dirty_material;
    std::vector<RenderObjectId> m_dirty_skeleton;
};

} // namespace cave::render
