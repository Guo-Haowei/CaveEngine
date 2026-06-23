#include "cave/runtime/scene/SceneCommandBuffer.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

using ecs::Entity;

SceneCommandExecutor::SceneCommandExecutor(Scene& p_scene, ecs::ComponentRegistry& p_reg) noexcept
    : m_scene(p_scene)
    , m_reg(p_reg) {
}

SceneCommandExecutor::SceneCommandExecutor(Scene& p_scene) noexcept
    : m_scene(p_scene)
    , m_reg(engine::GetComponentRegistry()) {
}

void SceneCommandExecutor::AddComponent(Entity p_ent, ComponentId p_id) {
    m_scene.storage().CreateRaw(p_ent, p_id);
    return;
}

bool SceneCommandExecutor::RemoveComponent(Entity p_ent, ComponentId p_id) {
    return m_scene.storage().Remove(p_ent, p_id);
}

bool SceneCommandExecutor::ChangeProperty(Entity p_ent,
                                          ComponentId p_cid,
                                          const PropertyId& p_pid,
                                          const void* p_data,
                                          uint32_t p_data_size) {
    const ecs::ComponentMeta* meta = m_reg.TryGet(p_cid);
    if (!meta) {
        LOG_WARN("Can't find meta for component '{}'", p_cid);
        return false;
    }

    void* comp = m_scene.storage().GetRaw(p_ent, meta->cid);
    if (!comp) {
        LOG_WARN("Can't find '{}' for ent {}", meta->name, p_ent.GetId());
        return false;
    }

    const FieldMetaBase* field = meta->Find(p_pid);
    if (!field) {
        LOG_WARN("Can't find '{}.{}' for ent {}", meta->name, p_pid.DebugName(), p_ent.GetId());
        return false;
    }

    char* data = reinterpret_cast<char*>(comp) + field->offset;
    std::memcpy(data, p_data, p_data_size);
    if (meta->on_edited) {
        meta->on_edited(m_scene, p_ent, p_cid, p_pid, p_data, p_data_size);
    }

    return true;
}

}  // namespace cave
