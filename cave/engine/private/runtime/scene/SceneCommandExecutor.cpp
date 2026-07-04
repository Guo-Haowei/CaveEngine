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

void SceneCommandExecutor::AddComponent(Entity ent, ComponentId cid) {
    m_scene.storage().createRaw(cid, ent);
    return;
}

bool SceneCommandExecutor::RemoveComponent(Entity ent, ComponentId cid) {
    return m_scene.storage().remove(cid, ent);
}

bool SceneCommandExecutor::ChangeProperty(Entity ent,
                                          ComponentId cid,
                                          const PropertyId& pid,
                                          const void* data,
                                          uint32_t data_size) {
    const ecs::ComponentMeta* meta = m_reg.TryGet(cid);
    if (!meta) {
        LOG_WARN("Can't find meta for component '{}'", cid);
        return false;
    }

    void* comp = m_scene.storage().getRaw(meta->cid, ent);
    if (!comp) {
        LOG_WARN("Can't find '{}' for ent {}", meta->name, ent.GetId());
        return false;
    }

    const FieldMetaBase* field = meta->Find(pid);
    if (!field) {
        LOG_WARN("Can't find '{}.{}' for ent {}", meta->name, pid.debugName(), ent.GetId());
        return false;
    }

    char* ptr = reinterpret_cast<char*>(comp) + field->offset;
    std::memcpy(ptr, data, data_size);
    if (meta->on_edited) {
        meta->on_edited(m_scene, ent, cid, pid, data, data_size);
    }

    return true;
}

}  // namespace cave
