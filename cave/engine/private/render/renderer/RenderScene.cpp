#include "RenderScene.h"

namespace cave::render {

RenderObjectId RenderScene::Ensure(ecs::Entity p_entity) {
    auto it = m_entity_to_id.find(p_entity);
    if (it != m_entity_to_id.end()) return it->second;

    RenderObjectId id = AllocSlot();
    m_entity_to_id.emplace(p_entity, id);

    RenderObject& o = m_objects[id];
    o = RenderObject{};
    o.entity = p_entity;

    MarkDirty(id, RENGER_DIRTY_FLAG_ALL);
    return id;
}

void RenderScene::Remove(ecs::Entity p_entity) {
    auto it = m_entity_to_id.find(p_entity);
    if (it == m_entity_to_id.end()) return;

    const RenderObjectId id = it->second;
    m_entity_to_id.erase(it);

    if (id < m_objects.size() && m_alive[id]) {
        m_alive[id] = false;
        m_free_ids.push_back(id);
    }
}

void RenderScene::MarkDirty(RenderObjectId p_id, RenderDirtyFlags p_dirty_flag) {
    if (p_id == kInvalidId) return;
    if (p_id >= m_objects.size() || !m_alive[p_id]) return;

    if (p_dirty_flag & RENGER_DIRTY_FLAG_TRANSFORM) m_dirty_transform.push_back(p_id);
    if (p_dirty_flag & RENGER_DIRTY_FLAG_MESH) m_dirty_mesh.push_back(p_id);
    if (p_dirty_flag & RENGER_DIRTY_FLAG_MATERIAL) m_dirty_material.push_back(p_id);
    if (p_dirty_flag & RENGER_DIRTY_FLAG_SKELETON) m_dirty_skeleton.push_back(p_id);
}

void RenderScene::ClearDirtyLists() {
    m_dirty_transform.clear();
    m_dirty_mesh.clear();
    m_dirty_material.clear();
    m_dirty_skeleton.clear();
}

void RenderScene::Reset() {
    m_objects.clear();
    m_alive.clear();
    m_free_ids.clear();
    m_entity_to_id.clear();
    ClearDirtyLists();
}

RenderObjectId RenderScene::AllocSlot() {
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

}  // namespace cave::render
