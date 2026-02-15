#include "cave/runtime/scene/SceneMutator.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

SceneMutator::SceneMutator(Scene& p_scene) noexcept
    : m_scene(p_scene)
    , m_reg(engine::GetComponentRegistry()) {
}

ecs::Entity SceneMutator::CreateEntity() {
    return m_scene.CreateEntity();
}

void SceneMutator::RemoveEntity(ecs::Entity p_ent) {
    m_scene.RemoveEntity(p_ent);
}

void* SceneMutator::AddComponent(ecs::Entity p_ent, ComponentId p_id) {
    return m_scene.Storage().CreateRaw(p_ent, p_id);
}

bool SceneMutator::RemoveComponent(ecs::Entity p_ent, ComponentId p_id) {
    return m_scene.Storage().Remove(p_ent, p_id);
}

bool SceneMutator::ChangeProperty(ecs::Entity p_ent,
                                  ComponentId p_comp_id,
                                  PropertyId p_property,
                                  const void* p_data,
                                  uint32_t p_data_size,
                                  void* p_old_data) {
    const ecs::ComponentMeta* meta = m_reg.TryGet(p_comp_id);
    if (!meta) {
        LOG_WARN("Can't find meta for component {}", p_comp_id);
        return false;
    }

    const FieldMetaBase* field = meta->Find(p_property);
    if (!field) {
        LOG_WARN("Can't find field '{}' for component {}", p_property, p_comp_id);
        return false;
    }

    void* comp = m_scene.Storage().GetRaw(p_ent, p_comp_id);
    char* data = reinterpret_cast<char*>(comp) + field->offset;
    if (p_old_data) {
        std::memcpy(p_old_data, data, p_data_size);
    }

    std::memcpy(data, p_data, p_data_size);
    if (meta->on_edited) {
        meta->on_edited(m_scene, p_ent, p_comp_id, p_property);
    }

    return true;
}

}  // namespace cave
