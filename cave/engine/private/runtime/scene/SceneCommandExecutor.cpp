#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneCommandExecutor.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/scene/Scene.h"

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

ecs::Entity SceneCommandExecutor::CreateEntity() {
    return m_scene.CreateEntity();
}

void SceneCommandExecutor::RemoveEntity(ecs::Entity p_ent) {
    m_scene.RemoveEntity(p_ent);
}

void* SceneCommandExecutor::AddComponent(ecs::Entity p_ent, ComponentId p_id) {
    return m_scene.Storage().CreateRaw(p_ent, p_id);
}

bool SceneCommandExecutor::RemoveComponent(ecs::Entity p_ent, ComponentId p_id) {
    return m_scene.Storage().Remove(p_ent, p_id);
}

bool SceneCommandExecutor::ChangeProperty(ecs::Entity p_ent,
                                          ComponentId p_cid,
                                          const PropertyId& p_pid,
                                          const void* p_data,
                                          uint32_t p_data_size) {

    const ecs::ComponentMeta* meta = m_reg.TryGet(p_cid);
    if (!meta) {
        LOG_WARN("Can't find meta for component {}", p_cid);
        return false;
    }

    const FieldMetaBase* field = meta->Find(p_pid);
    if (!field) {
        LOG_WARN("Can't find field '{}' for component {}", p_pid.GetHash(), p_cid);
        return false;
    }

    void* comp = m_scene.Storage().GetRaw(p_ent, p_cid);
    char* data = reinterpret_cast<char*>(comp) + field->offset;
    // if (p_old_data) {
    //     std::memcpy(p_old_data, data, p_data_size);
    // }

    std::memcpy(data, p_data, p_data_size);
    if (meta->on_edited) {
        meta->on_edited(m_scene, p_ent, p_cid, p_pid, p_data, p_data_size);
    }

    return true;
}

static bool IsTemp(ecs::Entity p_id) noexcept { return p_id.GetId() >= kSceneCmdTmpBase; }

Entity SceneCommandExecutor::Resolve(Entity p_ent) const noexcept {
    if (!IsTemp(p_ent)) return p_ent;

    const uint32_t index = p_ent.GetId() - kSceneCmdTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return Entity::Null();
}

void SceneCommandExecutor::SetRemap(Entity p_temp, Entity p_real) {
    const uint32_t index = p_temp.GetId() - kSceneCmdTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = p_real;
}

void SceneCommandExecutor::Playback(SceneCommandBuffer& p_cb) {
    const uint8_t* p = p_cb.Data();
    if (p == nullptr) {
        return;
    }

    const uint8_t* end = p + p_cb.Size();

    m_remap.clear();
    m_remap.resize(p_cb.GetAllocationCount(), Entity::Null());

    while (p < end) {
        const SceneCmd_Header* header = reinterpret_cast<const SceneCmd_Header*>(p);
        const uint8_t* payload_raw = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case SceneCmd_Op::CreateEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                Entity real = CreateEntity();
                SetRemap(e, real);
            } break;
            case SceneCmd_Op::DestroyEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                RemoveEntity(Resolve(e));
            } break;
            case SceneCmd_Op::AddComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                AddComponent(Resolve(payload->ent), payload->cid);
            } break;
            case SceneCmd_Op::RemoveComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                RemoveComponent(Resolve(payload->ent), payload->cid);
            } break;
            case SceneCmd_Op::AssignProperty: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadProperty*>(payload_raw);
                const void* data = reinterpret_cast<const void*>(payload + 1);

                switch (payload->ptype) {
                    case SceneCmd_PropType::PlainData: {
                    } break;
                    case SceneCmd_PropType::Entity: {
                        DEV_ASSERT(payload->data_size >= payload->ele_count * sizeof(Entity));
                        // @HACK: cast away const to resolve entity
                        Entity* e = const_cast<Entity*>((Entity*)data);
                        for (uint32_t i = 0; i < payload->ele_count; ++i) {
                            e[i] = Resolve(e[i]);
                        }
                    } break;
                    default: {
                        CRASH_NOW();
                    } break;
                }

                ChangeProperty(Resolve(payload->ent),
                               payload->cid,
                               payload->pid,
                               data,
                               payload->data_size);
            } break;
            default: {
                CRASH_NOW_MSG("Invalid opcode");
            } break;
        }
        p += header->size;
    }
}

}  // namespace cave
