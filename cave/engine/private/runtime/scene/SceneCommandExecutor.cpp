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

void SceneCommandExecutor::addComponent(Entity ent, ComponentId cid) {
    m_scene.storage().createRaw(cid, ent);
    return;
}

bool SceneCommandExecutor::removeComponent(Entity ent, ComponentId cid) {
    return m_scene.storage().remove(cid, ent);
}

bool SceneCommandExecutor::changeProperty(Entity ent,
                                          ComponentId cid,
                                          const PropertyId& pid,
                                          const void* new_value,
                                          uint32_t data_size) {
    const ecs::ComponentMeta* meta = m_reg.tryGet(cid);
    if (!meta) {
        LOG_WARN("Can't find meta for component '{}'", cid);
        return false;
    }

    void* comp = m_scene.storage().getRaw(meta->cid, ent);
    if (!comp) {
        LOG_WARN("Can't find '{}' for ent {}", meta->name, ent.id());
        return false;
    }

    const FieldMetaBase* field = meta->find(pid);
    if (!field) {
        LOG_WARN("Can't find '{}.{}' for ent {}", meta->name, pid.debugName(), ent.id());
        return false;
    }

    void* ptr = field->getRaw(comp);
    if (memcmp(ptr, new_value, data_size) == 0) [[unlikely]] {
        return false;
    }

    Vector<uint8_t> old_value(data_size, 0);
    std::memcpy(old_value.data(), ptr, data_size);
    std::memcpy(ptr, new_value, data_size);
    if (meta->on_edited) {
        meta->on_edited(m_scene, ent, cid, pid, new_value, data_size);
    }

    if (field->on_change) {
        FieldChange change {
            .scene = &m_scene,
            .entity = ent,
            .object = comp,
            .field = field,
            .old_value = old_value.data(),
            .new_value = new_value,
        };
        field->on_change(change);
    }

    return true;
}

}  // namespace cave
